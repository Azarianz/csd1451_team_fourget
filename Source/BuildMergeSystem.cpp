#include "BuildMergeSystem.h"

#include <algorithm>
#include <cmath>

void BuildMergeSystem::Init(LevelLoader* levelPtr,
    GridSystem::Grid* gridPtr,
    TowerHandler::Shop* shopPtr,
    std::vector<TowerHandler::Tower>* towersPtr,
    std::vector<TowerHandler::ActiveBullet>* bulletsPtr,
    std::vector<uint8_t>* occupiedPtr)
{
    level = levelPtr;
    grid = gridPtr;
    shop = shopPtr;
    activeTowers = towersPtr;
    activeBullets = bulletsPtr;
    occupied = occupiedPtr;

    BuildXMeshIfNeeded();
}

void BuildMergeSystem::Shutdown()
{
    FreeXMesh();

    level = nullptr;
    grid = nullptr;
    shop = nullptr;
    activeTowers = nullptr;
    activeBullets = nullptr;
    occupied = nullptr;
}

int BuildMergeSystem::Idx(int x, int y) const
{
    return level ? level->Idx(x, y) : -1;
}

bool BuildMergeSystem::InBounds(int x, int y) const
{
    return level && x >= 0 && y >= 0 && x < level->width && y < level->height;
}

bool BuildMergeSystem::IsBuildable(int x, int y) const
{
    if (!level) return false;

    int i = Idx(x, y);
    if (i < 0) return false;
    if ((size_t)i >= level->region.size()) return false;

    return level->region[(size_t)i] == 1;
}

bool BuildMergeSystem::IsOccupied(int x, int y) const
{
    if (!occupied) return false;

    int i = Idx(x, y);
    if (i < 0) return false;
    if ((size_t)i >= occupied->size()) return false;

    return (*occupied)[(size_t)i] != 0;
}

bool BuildMergeSystem::IsPlaceable(int x, int y) const
{
    return IsBuildable(x, y) && !IsOccupied(x, y);
}

int BuildMergeSystem::FindDraggedTowerIndex() const
{
    if (!activeTowers) return -1;

    for (int i = (int)activeTowers->size() - 1; i >= 0; --i)
    {
        if ((*activeTowers)[(size_t)i].isDragging)
            return i;
    }

    return -1;
}

bool BuildMergeSystem::IsDraggingTower() const
{
    return FindDraggedTowerIndex() >= 0;
}

void BuildMergeSystem::UpdateDragging(float worldX, float worldY,
    bool lmbDown, bool justReleasedLmb,
    int mouseX, int mouseY)
{
    if (!activeTowers)
        return;

    if (justReleasedLmb && IsDraggingTower())
    {
        SnapDraggedTowerToGrid(mouseX, mouseY);
    }

    int draggedIndex = FindDraggedTowerIndex();
    if (draggedIndex >= 0 && lmbDown)
    {
        (*activeTowers)[(size_t)draggedIndex].x = worldX;
        (*activeTowers)[(size_t)draggedIndex].y = worldY;
    }
}

void BuildMergeSystem::RebuildOccupiedFromTowers()
{
    if (!level || !occupied || !activeTowers)
        return;

    const size_t expected = (size_t)level->width * (size_t)level->height;
    occupied->assign(expected, 0);

    for (const auto& t : *activeTowers)
    {
        GridSystem::GridCoord c;
        if (GetTowerCell(t, c) && InBounds(c.x, c.y))
        {
            (*occupied)[(size_t)Idx(c.x, c.y)] = 1;
        }
    }
}

bool BuildMergeSystem::GetTowerCell(const TowerHandler::Tower& t, GridSystem::GridCoord& outCell) const
{
    if (!grid) return false;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    int mouseX = (int)(t.x + screenW * 0.5f);
    int mouseY = (int)(screenH * 0.5f - t.y);

    return grid->ScreenToGrid(mouseX, mouseY, outCell);
}

int BuildMergeSystem::FindTowerIndexAtCell(int x, int y) const
{
    if (!activeTowers) return -1;

    for (int i = 0; i < (int)activeTowers->size(); ++i)
    {
        GridSystem::GridCoord c;
        if (GetTowerCell((*activeTowers)[(size_t)i], c) && c.x == x && c.y == y)
            return i;
    }

    return -1;
}

bool BuildMergeSystem::TowerMatchesAtCell(int x, int y, TowerHandler::TowerType type, int towerLevel) const
{
    if (!InBounds(x, y) || !activeTowers)
        return false;

    int idx = FindTowerIndexAtCell(x, y);
    if (idx < 0)
        return false;

    const TowerHandler::Tower& t = (*activeTowers)[(size_t)idx];
    return t.details.towerType == type && t.details.level == towerLevel;
}

bool BuildMergeSystem::TryMergeAtCell(int placedX, int placedY)
{
    if (!activeTowers)
        return false;

    int centerIndex = FindTowerIndexAtCell(placedX, placedY);
    if (centerIndex < 0)
        return false;

    TowerHandler::TowerType type = (*activeTowers)[(size_t)centerIndex].details.towerType;
    int towerLevel = (*activeTowers)[(size_t)centerIndex].details.level;

    if (type == TowerHandler::BASE_TOWER)
        return false;

    std::vector<GridSystem::GridCoord> mergeCells;

    if (TowerMatchesAtCell(placedX - 1, placedY, type, towerLevel) &&
        TowerMatchesAtCell(placedX + 1, placedY, type, towerLevel))
    {
        mergeCells.push_back({ placedX - 1, placedY });
        mergeCells.push_back({ placedX,     placedY });
        mergeCells.push_back({ placedX + 1, placedY });
    }
    else if (TowerMatchesAtCell(placedX - 2, placedY, type, towerLevel) &&
        TowerMatchesAtCell(placedX - 1, placedY, type, towerLevel))
    {
        mergeCells.push_back({ placedX - 2, placedY });
        mergeCells.push_back({ placedX - 1, placedY });
        mergeCells.push_back({ placedX,     placedY });
    }
    else if (TowerMatchesAtCell(placedX + 1, placedY, type, towerLevel) &&
        TowerMatchesAtCell(placedX + 2, placedY, type, towerLevel))
    {
        mergeCells.push_back({ placedX,     placedY });
        mergeCells.push_back({ placedX + 1, placedY });
        mergeCells.push_back({ placedX + 2, placedY });
    }

    if (mergeCells.empty())
    {
        if (TowerMatchesAtCell(placedX, placedY - 1, type, towerLevel) &&
            TowerMatchesAtCell(placedX, placedY + 1, type, towerLevel))
        {
            mergeCells.push_back({ placedX, placedY - 1 });
            mergeCells.push_back({ placedX, placedY });
            mergeCells.push_back({ placedX, placedY + 1 });
        }
        else if (TowerMatchesAtCell(placedX, placedY - 2, type, towerLevel) &&
            TowerMatchesAtCell(placedX, placedY - 1, type, towerLevel))
        {
            mergeCells.push_back({ placedX, placedY - 2 });
            mergeCells.push_back({ placedX, placedY - 1 });
            mergeCells.push_back({ placedX, placedY });
        }
        else if (TowerMatchesAtCell(placedX, placedY + 1, type, towerLevel) &&
            TowerMatchesAtCell(placedX, placedY + 2, type, towerLevel))
        {
            mergeCells.push_back({ placedX, placedY });
            mergeCells.push_back({ placedX, placedY + 1 });
            mergeCells.push_back({ placedX, placedY + 2 });
        }
    }

    if (mergeCells.size() != 3)
        return false;

    std::vector<int> toRemove;
    for (const auto& cell : mergeCells)
    {
        if (cell.x == placedX && cell.y == placedY)
            continue;

        int idx = FindTowerIndexAtCell(cell.x, cell.y);
        if (idx >= 0)
            toRemove.push_back(idx);
    }

    (*activeTowers)[(size_t)centerIndex].LevelUp();

    std::sort(toRemove.begin(), toRemove.end());
    toRemove.erase(std::unique(toRemove.begin(), toRemove.end()), toRemove.end());

    for (int i = (int)toRemove.size() - 1; i >= 0; --i)
    {
        int idx = toRemove[(size_t)i];

        if (idx < centerIndex)
            --centerIndex;

        RemoveTowerAtIndex(idx);
    }

    return true;
}

void BuildMergeSystem::RemoveTowerAtIndex(int idx)
{
    if (!activeTowers || idx < 0 || idx >= (int)activeTowers->size())
        return;

    TowerHandler::Tower& t = (*activeTowers)[(size_t)idx];

    if (t.spriteId != 0)
    {
        Graphics::Destroy(t.spriteId);
        t.spriteId = 0;
    }

    t.Destroy();

    activeTowers->erase(activeTowers->begin() + idx);

    if (activeBullets)
        activeBullets->clear();
}

bool BuildMergeSystem::SnapDraggedTowerToGrid(int mouseX, int mouseY)
{
    if (!grid || !shop || !activeTowers)
        return false;

    int draggedIndex = FindDraggedTowerIndex();
    if (draggedIndex < 0)
        return false;

    GridSystem::GridCoord c;
    if (!grid->ScreenToGrid(mouseX, mouseY, c) || !InBounds(c.x, c.y))
    {
        float slotX = 0.f, slotY = 0.f;
        int slotIdx = (*activeTowers)[(size_t)draggedIndex].sourceSlotIndex;

        if (shop->GetSlotCenter(slotIdx, slotX, slotY))
        {
            (*activeTowers)[(size_t)draggedIndex].isDragging = false;
            (*activeTowers)[(size_t)draggedIndex].isReturning = true;
            (*activeTowers)[(size_t)draggedIndex].returnTargetX = slotX;
            (*activeTowers)[(size_t)draggedIndex].returnTargetY = slotY;
        }
        else
        {
            RemoveTowerAtIndex(draggedIndex);
            RebuildOccupiedFromTowers();
        }

        return false;
    }

    if (!IsPlaceable(c.x, c.y))
    {
        float slotX = 0.f, slotY = 0.f;
        int slotIdx = (*activeTowers)[(size_t)draggedIndex].sourceSlotIndex;

        if (shop->GetSlotCenter(slotIdx, slotX, slotY))
        {
            (*activeTowers)[(size_t)draggedIndex].isDragging = false;
            (*activeTowers)[(size_t)draggedIndex].isReturning = true;
            (*activeTowers)[(size_t)draggedIndex].returnTargetX = slotX;
            (*activeTowers)[(size_t)draggedIndex].returnTargetY = slotY;
        }
        else
        {
            RemoveTowerAtIndex(draggedIndex);
            RebuildOccupiedFromTowers();
        }

        return false;
    }

    float wx = 0.f, wy = 0.f, tileSize = 0.f;
    grid->GetCellWorldCenter({ c.x, c.y }, wx, wy, tileSize);

    (*activeTowers)[(size_t)draggedIndex].x = wx;
    (*activeTowers)[(size_t)draggedIndex].y = wy;
    (*activeTowers)[(size_t)draggedIndex].isDragging = false;
    (*activeTowers)[(size_t)draggedIndex].isSelected = false;

    RebuildOccupiedFromTowers();

    while (TryMergeAtCell(c.x, c.y))
    {
        RebuildOccupiedFromTowers();
    }

    RebuildOccupiedFromTowers();
    return true;
}

void BuildMergeSystem::BuildXMeshIfNeeded()
{
    if (xMesh) return;

    const float half = 0.45f;
    const float thick = 0.06f;

    auto AddQuad = [](float x0, float y0, float x1, float y1, float t)
        {
            float dx = x1 - x0, dy = y1 - y0;
            float len = std::sqrt(dx * dx + dy * dy);
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

void BuildMergeSystem::FreeXMesh()
{
    if (xMesh)
    {
        AEGfxMeshFree(xMesh);
        xMesh = nullptr;
    }
}

void BuildMergeSystem::DrawXAtCell(int x, int y, float alpha)
{
    if (!xMesh || !grid) return;

    float wx = 0.0f, wy = 0.0f, tileSize = 0.0f;
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

void BuildMergeSystem::DrawOverlay()
{
    if (!grid || !level || !occupied)
        return;

    const size_t expected = (size_t)level->width * (size_t)level->height;
    if (expected == 0 || occupied->size() != expected)
        return;

    grid->Draw();

    for (int y = 0; y < level->height; ++y)
    {
        for (int x = 0; x < level->width; ++x)
        {
            if (!IsPlaceable(x, y))
                DrawXAtCell(x, y, 0.75f);
        }
    }

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
