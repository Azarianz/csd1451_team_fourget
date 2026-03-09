#include "Scene_Prototype.h"

#include "AEEngine.h"
#include "AEInput.h"
#include "Utility.h"   // IMPORTANT (for GetWorldMousePos)
#include "LevelData.h"
#include <cstdio>
#include <cmath>
#include <algorithm>

// ----------------------------------------------------
// Drag helpers
// ----------------------------------------------------
int Scene_Prototype::FindDraggedTowerIndex() const
{
    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
        if (activeTowers[i].isDragging)
            return i;
    return -1;
}

bool Scene_Prototype::IsDraggingTower() const
{
    return FindDraggedTowerIndex() >= 0;
}

// ----------------------------------------------------
// Merge System
// ----------------------------------------------------
void Scene_Prototype::RebuildOccupiedFromTowers()
{
    const size_t expected = (size_t)level.width * (size_t)level.height;
    occupied.assign(expected, 0);

    for (const auto& t : activeTowers)
    {
        GridSystem::GridCoord c;
        if (GetTowerCell(t, c) && InBounds(c.x, c.y))
        {
            occupied[Idx(c.x, c.y)] = 1;
        }
    }
}

bool Scene_Prototype::GetTowerCell(const TowerHandler::Tower& t, GridSystem::GridCoord& outCell) const
{
    if (!grid) return false;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    // convert tower world position back to screen pixels
    int mouseX = (int)(t.x + screenW * 0.5f);
    int mouseY = (int)(screenH * 0.5f - t.y);

    return grid->ScreenToGrid(mouseX, mouseY, outCell);
}

int Scene_Prototype::FindTowerIndexAtCell(int x, int y) const
{
    for (int i = 0; i < (int)activeTowers.size(); ++i)
    {
        GridSystem::GridCoord c;
        if (GetTowerCell(activeTowers[i], c) && c.x == x && c.y == y)
            return i;
    }
    return -1;
}

void Scene_Prototype::CollectConnectedMergeGroup(
    int startX,
    int startY,
    TowerHandler::TowerType type,
    int towerLevel,
    std::vector<GridSystem::GridCoord>& outGroup) const
{
    outGroup.clear();

    if (!InBounds(startX, startY))
        return;

    const int total = this->level.width * this->level.height;
    if (total <= 0)
        return;

    std::vector<uint8_t> visited((size_t)total, 0);
    std::vector<GridSystem::GridCoord> open;

    open.push_back({ startX, startY });

    while (!open.empty())
    {
        GridSystem::GridCoord cur = open.back();
        open.pop_back();

        if (!InBounds(cur.x, cur.y))
            continue;

        int idx = Idx(cur.x, cur.y);
        if (visited[(size_t)idx])
            continue;

        visited[(size_t)idx] = 1;

        int towerIndex = FindTowerIndexAtCell(cur.x, cur.y);
        if (towerIndex < 0)
            continue;

        const TowerHandler::Tower& t = activeTowers[(size_t)towerIndex];
        if (t.details.towerType != type || t.details.level != towerLevel)
            continue;

        outGroup.push_back(cur);

        open.push_back({ cur.x + 1, cur.y });
        open.push_back({ cur.x - 1, cur.y });
        open.push_back({ cur.x, cur.y + 1 });
        open.push_back({ cur.x, cur.y - 1 });
    }
}

bool Scene_Prototype::TryMergeAtCell(int placedX, int placedY)
{
    int centerIndex = FindTowerIndexAtCell(placedX, placedY);
    if (centerIndex < 0)
        return false;

    TowerHandler::Tower& placedTower = activeTowers[(size_t)centerIndex];
    TowerHandler::TowerType type = placedTower.details.towerType;
    int level = placedTower.details.level;

    std::vector<GridSystem::GridCoord> group;
    CollectConnectedMergeGroup(placedX, placedY, type, level, group);

    // merge rule: at least 3 connected matching towers
    if (group.size() < 3)
        return false;

    // keep the newly placed tower, remove the others
    std::vector<int> toRemove;
    for (const auto& cell : group)
    {
        if (cell.x == placedX && cell.y == placedY)
            continue;

        int idx = FindTowerIndexAtCell(cell.x, cell.y);
        if (idx >= 0)
            toRemove.push_back(idx);
    }

    // level up the placed tower first
    activeTowers[(size_t)centerIndex].LevelUp();

    // erase from back to front
    std::sort(toRemove.begin(), toRemove.end());
    toRemove.erase(std::unique(toRemove.begin(), toRemove.end()), toRemove.end());

    for (int i = (int)toRemove.size() - 1; i >= 0; --i)
    {
        int idx = toRemove[(size_t)i];

        // if a removed index is before centerIndex, centerIndex shifts left
        if (idx < centerIndex)
            --centerIndex;

        activeTowers.erase(activeTowers.begin() + idx);
    }

    return true;
}

// ----------------------------------------------------
// Level loading
// ----------------------------------------------------
bool Scene_Prototype::LoadLevel(int idx)
{
    sprintf_s(levelPath, "Assets/Levels/level_%02d.txt", idx);

    if (grid)
    {
        grid->Destroy();    
        delete grid;
        grid = nullptr;
    }
    level.Shutdown();

    if (!level.LoadFromText(levelPath))
    {
        PRINT("FAILED to load level: %s\n", levelPath);
        return false;
    }

    const size_t expected = (size_t)level.width * (size_t)level.height;
    if (expected == 0 ||
        level.map.size() != expected ||
        level.region.size() != expected)
    {
        PRINT("LEVEL DATA SIZE MISMATCH! w=%d h=%d expected=%zu map=%zu region=%zu\n",
            level.width, level.height,
            expected, level.map.size(), level.region.size());
        return false;
    }

    occupied.assign(expected, 0);

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    path.clear();
    if (!BuildPathFromRegionScan())
    {
        PRINT("FAILED to build enemy path from region flags.\n");
        return false;
    }

    return true;
}

bool Scene_Prototype::BuildPathFromRegionScan()
{
    path.clear();

    if (!grid) return false;
    if (level.width <= 0 || level.height <= 0) return false;

    GridSystem::GridCoord spawn{};
    if (!FindSpawnCell(spawn))
    {
        PRINT("No ENEMYSPAWN found.\n");
        return false;
    }

    // first point = spawn center
    PushCellCenterToPath(spawn);

    GridSystem::GridCoord current = spawn;
    GridSystem::GridCoord next{};

    if (!FindNextFromSpawn(spawn, next))
    {
        PRINT("Spawn does not connect to any enemy path or goal.\n");
        return false;
    }

    // safety against infinite loops
    const int maxSteps = level.width * level.height + 8;

    for (int step = 0; step < maxSteps; ++step)
    {
        current = next;
        PushCellCenterToPath(current);

        RegionFlag flag = static_cast<RegionFlag>(level.region[Idx(current.x, current.y)]);

        if (flag == RegionFlag::ENEMYGOAL)
        {
            PRINT("Enemy path built. Total points: %d\n", (int)path.size());
            return true;
        }

        if (!StepFromRegionFlag(current, next))
        {
            PRINT("Broken enemy path at cell (%d, %d)\n", current.x, current.y);
            return false;
        }
    }

    PRINT("Enemy path exceeded safety limit. Possible loop.\n");
    return false;
}

bool Scene_Prototype::FindSpawnCell(GridSystem::GridCoord& outSpawn) const
{
    for (int y = 0; y < level.height; ++y)
    {
        for (int x = 0; x < level.width; ++x)
        {
            RegionFlag flag = static_cast<RegionFlag>(level.region[Idx(x, y)]);
            if (flag == RegionFlag::ENEMYSPAWN)
            {
                outSpawn = { x, y };
                return true;
            }
        }
    }

    return false;
}

bool Scene_Prototype::FindNextFromSpawn(const GridSystem::GridCoord& spawn, GridSystem::GridCoord& outNext) const
{
    const GridSystem::GridCoord neighbors[4] =
    {
        { spawn.x,     spawn.y - 1 }, // up
        { spawn.x,     spawn.y + 1 }, // down
        { spawn.x - 1, spawn.y     }, // left
        { spawn.x + 1, spawn.y     }  // right
    };

    for (const auto& n : neighbors)
    {
        if (!InBounds(n.x, n.y))
            continue;

        RegionFlag flag = static_cast<RegionFlag>(level.region[Idx(n.x, n.y)]);

        if (flag == RegionFlag::ENEMYGOAL ||
            flag == RegionFlag::ENEMYPATH_UP ||
            flag == RegionFlag::ENEMYPATH_DOWN ||
            flag == RegionFlag::ENEMYPATH_LEFT ||
            flag == RegionFlag::ENEMYPATH_RIGHT)
        {
            outNext = n;
            return true;
        }
    }

    return false;
}

bool Scene_Prototype::StepFromRegionFlag(const GridSystem::GridCoord& current, GridSystem::GridCoord& outNext) const
{
    RegionFlag flag = static_cast<RegionFlag>(level.region[Idx(current.x, current.y)]);
    outNext = current;

    switch (flag)
    {
    case RegionFlag::ENEMYPATH_UP:
        outNext.y -= 1;
        break;

    case RegionFlag::ENEMYPATH_DOWN:
        outNext.y += 1;
        break;

    case RegionFlag::ENEMYPATH_LEFT:
        outNext.x -= 1;
        break;

    case RegionFlag::ENEMYPATH_RIGHT:
        outNext.x += 1;
        break;

    default:
        return false;
    }

    if (!InBounds(outNext.x, outNext.y))
        return false;

    return true;
}

void Scene_Prototype::PushCellCenterToPath(const GridSystem::GridCoord& cell)
{
    float wx = 0.0f;
    float wy = 0.0f;
    float tileSize = 0.0f;
    grid->GetCellWorldCenter(cell, wx, wy, tileSize);
    path.push_back({ wx, wy });
}

// ----------------------------------------------------
// Scene lifecycle
// ----------------------------------------------------
void Scene_Prototype::Init()
{
    if (!LoadLevel(levelIndex))
    {
        PRINT("Scene_Prototype Init failed to load level.\n");
        return;
    }

    shop.Init();
    activeTowers.clear();

    BuildXMeshIfNeeded();

    enemies.clear();
    spawnTimer = 0.0f;

    buildMode = false;
    wasLmbDown = false;
}

void Scene_Prototype::Update(float dt)
{
    if (!grid) return;

    // --- Mouse: get BOTH screen + world ---
    int mouseX = 0, mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float worldX = 0.0f, worldY = 0.0f;
    Utility::GetWorldMousePos(worldX, worldY);

    // 1) shop may spawn tower and set isDragging
    shop.Update(activeTowers);

    // 2) build mode based on tower flags BEFORE moving anything
    buildMode = IsDraggingTower();

    // 3) release-to-place first (if released this frame, we place immediately)
    bool lmbDown = AEInputCheckCurr(AEVK_LBUTTON);
    bool justReleased = (wasLmbDown && !lmbDown);

    if (buildMode && justReleased)
    {
        SnapDraggedTowerToGrid(mouseX, mouseY);
    }

    // 4) Only run drag system if we are STILL holding (still dragging)
    buildMode = IsDraggingTower(); // refresh after snap
    if (buildMode && lmbDown)
    {
        TowerHandler::ShopTower dummyShop;
        TowerHandler::UpdateTowerSystem(worldX, worldY, dummyShop, activeTowers);
    }

    wasLmbDown = lmbDown;

    // --- Enemy spawning ---
    spawnTimer += dt;
    if (spawnTimer >= spawnInterval && !path.empty())
    {
        spawnTimer = 0.0f;

        Enemy e;
        e.Init(35.0f, 35.0f, Color{ 1,0,0,1 }, 50.0f, 5.0f, 120.0f);

        e.x = path[0].x;
        e.y = path[0].y;

        enemies.push_back(e);
    }

    for (auto& e : enemies)
        e.Update(dt, path);
}

void Scene_Prototype::Draw()
{
    if (!grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    // level
    level.Draw(*grid, 1.0f);

    // towers
    for (auto& t : activeTowers)
        t.Draw();

    // enemies
    for (auto& e : enemies)
        e.Draw();

    // build overlay
    if (buildMode)
        DrawBuildOverlay();

    // shop UI always on top (like ShopTest)
    shop.Draw();
}

void Scene_Prototype::Exit()
{
    shop.Exit();
    activeTowers.clear();

    if (xMesh)
    {
        AEGfxMeshFree(xMesh);
        xMesh = nullptr;
    }

    level.Shutdown();

    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
}

// ----------------------------------------------------
// Snap-to-grid on release
// ----------------------------------------------------
void Scene_Prototype::SnapDraggedTowerToGrid(int mouseX, int mouseY)
{
    int draggedIndex = FindDraggedTowerIndex();
    if (draggedIndex < 0) return;

    GridSystem::GridCoord c;
    if (!grid->ScreenToGrid(mouseX, mouseY, c) || !InBounds(c.x, c.y))
    {
        // released off-grid -> cancel tower
        activeTowers.erase(activeTowers.begin() + draggedIndex);
        RebuildOccupiedFromTowers();
        return;
    }

    if (!IsPlaceable(c.x, c.y))
    {
        // not placeable -> cancel tower
        activeTowers.erase(activeTowers.begin() + draggedIndex);
        RebuildOccupiedFromTowers();
        return;
    }

    // snap to cell center
    float wx, wy, tileSize;
    grid->GetCellWorldCenter({ c.x, c.y }, wx, wy, tileSize);

    activeTowers[draggedIndex].x = wx;
    activeTowers[draggedIndex].y = wy;
    activeTowers[draggedIndex].isDragging = false;
    activeTowers[draggedIndex].isSelected = false;

    RebuildOccupiedFromTowers();

    // try merge after placement
    TryMergeAtCell(c.x, c.y);

    RebuildOccupiedFromTowers();
}

// ----------------------------------------------------
// Build overlay rendering
// ----------------------------------------------------
void Scene_Prototype::BuildXMeshIfNeeded()
{
    if (xMesh) return;

    const float half = 0.45f;
    const float thick = 0.06f;

    auto AddQuad = [](float x0, float y0, float x1, float y1, float t)
        {
            float dx = x1 - x0, dy = y1 - y0;
            float len = sqrtf(dx * dx + dy * dy);
            if (len <= 0.0001f) return;

            dx /= len; dy /= len;
            float px = -dy * t;
            float py = dx * t;

            float ax = x0 + px, ay = y0 + py;
            float bx = x0 - px, by = y0 - py;
            float cx = x1 - px, cy = y1 - py;
            float dx2 = x1 + px, dy2 = y1 + py;

            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, bx, by, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0);
            AEGfxTriAdd(ax, ay, 0xFFFFFFFF, 0, 0, cx, cy, 0xFFFFFFFF, 0, 0, dx2, dy2, 0xFFFFFFFF, 0, 0);
        };

    AEGfxMeshStart();
    AddQuad(-half, -half, half, half, thick);
    AddQuad(-half, half, half, -half, thick);
    xMesh = AEGfxMeshEnd();
}

void Scene_Prototype::DrawXAtCell(int x, int y, float alpha)
{
    if (!xMesh || !grid) return;

    float wx, wy, tileSize;
    grid->GetCellWorldCenter({ x, y }, wx, wy, tileSize);

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0, 0, 0, 0);
    AEGfxSetColorToMultiply(1, 1, 1, 1);
    AEGfxSetTransparency(alpha);

    AEMtx33 scale, trans, m;
    AEMtx33Scale(&scale, tileSize, tileSize);
    AEMtx33Trans(&trans, wx, wy);
    AEMtx33Concat(&m, &trans, &scale);

    AEGfxSetTransform(m.m);
    AEGfxMeshDraw(xMesh, AE_GFX_MDM_TRIANGLES);

    AEGfxSetTransparency(1.0f);
}

void Scene_Prototype::DrawBuildOverlay()
{
    if (!grid) return;

    const size_t expected = (size_t)level.width * (size_t)level.height;
    if (expected == 0 || occupied.size() != expected)
        return;

    grid->Draw();

    // X on non-placeable tiles
    for (int y = 0; y < level.height; ++y)
        for (int x = 0; x < level.width; ++x)
            if (!IsPlaceable(x, y))
                DrawXAtCell(x, y, 0.75f);

    // hover tint uses current screen mouse
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (IsPlaceable(c.x, c.y))
            grid->DrawTileTinted(c, 0.2f, 1.0f, 0.2f, 0.55f);
        else
            grid->DrawTileTinted(c, 1.0f, 0.2f, 0.2f, 0.55f);
    }
}