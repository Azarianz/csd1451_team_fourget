#include "Scene_Prototype.h"

#include "AEEngine.h"
#include "AEInput.h"
#include "Utility.h"   // IMPORTANT (for GetWorldMousePos)
#include <cstdio>
#include <cmath>

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
    BuildPathFromRegionScan();

    return true;
}

void Scene_Prototype::BuildPathFromRegionScan()
{
    if (!grid) return;

    for (int y = 0; y < level.height; ++y)
    {
        for (int x = 0; x < level.width; ++x)
        {
            // REGION == 2 means enemy path
            if (level.region[Idx(x, y)] == 2)
            {
                float wx, wy, tileSize;
                grid->GetCellWorldCenter({ x, y }, wx, wy, tileSize);
                path.push_back({ wx, wy });
            }
        }
    }
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
        return;
    }

    if (!IsPlaceable(c.x, c.y))
    {
        // not placeable -> cancel tower
        activeTowers.erase(activeTowers.begin() + draggedIndex);
        return;
    }

    // snap to CELL CENTER (world coords)
    float wx, wy, tileSize;
    grid->GetCellWorldCenter({ c.x, c.y }, wx, wy, tileSize);

    activeTowers[draggedIndex].x = wx;
    activeTowers[draggedIndex].y = wy;

    // STOP dragging so UpdateTowerSystem won't override next frame
    activeTowers[draggedIndex].isDragging = false;
    activeTowers[draggedIndex].isSelected = false;

    occupied[Idx(c.x, c.y)] = 1;
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