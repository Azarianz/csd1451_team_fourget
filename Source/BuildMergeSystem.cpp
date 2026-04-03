#include "BuildMergeSystem.h"

#include <algorithm>
#include <cmath>

/*
===============================================================================
Initializes the build/merge system by storing pointers to all required systems.

levelPtr      -> level data (grid size, buildable region info)
gridPtr       -> grid conversion + drawing helper
shopPtr       -> shop system for returning dragged towers to slot
towersPtr     -> all currently active towers
bulletsPtr    -> all currently active bullets
occupiedPtr   -> occupancy map for which cells are occupied by towers
===============================================================================
*/
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

    // Build the reusable X mesh used for blocked cells in the overlay
    BuildXMeshIfNeeded();
}

/*
===============================================================================
Shuts down the system and clears all stored pointers.
Also frees the overlay X mesh.
===============================================================================
*/
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

/*
===============================================================================
Converts grid coordinates into a 1D array index.

Example:
(x, y) -> y * width + x
(depending on how LevelLoader::Idx is implemented)
===============================================================================
*/
int BuildMergeSystem::Idx(int x, int y) const
{
    return level ? level->Idx(x, y) : -1;
}

/*
===============================================================================
Checks if a grid cell is inside the level boundaries.
===============================================================================
*/
bool BuildMergeSystem::InBounds(int x, int y) const
{
    return level && x >= 0 && y >= 0 && x < level->width && y < level->height;
}

/*
===============================================================================
Returns true if the cell is marked as buildable in the level region map.

A region value of 1 means tower placement is allowed.
===============================================================================
*/
bool BuildMergeSystem::IsBuildable(int x, int y) const
{
    if (!level) return false;

    int i = Idx(x, y);
    if (i < 0) return false;
    if ((size_t)i >= level->region.size()) return false;

    return level->region[(size_t)i] == 1;
}

/*
===============================================================================
Checks whether a grid cell is currently occupied by a tower.
===============================================================================
*/
bool BuildMergeSystem::IsOccupied(int x, int y) const
{
    if (!occupied) return false;

    int i = Idx(x, y);
    if (i < 0) return false;
    if ((size_t)i >= occupied->size()) return false;

    return (*occupied)[(size_t)i] != 0;
}

/*
===============================================================================
A tile is placeable if:
1) it is buildable
2) it is not already occupied
===============================================================================
*/
bool BuildMergeSystem::IsPlaceable(int x, int y) const
{
    return IsBuildable(x, y) && !IsOccupied(x, y);
}

/*
===============================================================================
Finds the currently dragged tower.

Searches backwards so if multiple towers somehow overlap, the later / topmost
one in the vector gets priority.
===============================================================================
*/
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

/*
===============================================================================
Returns true if there is currently a tower being dragged.
===============================================================================
*/
bool BuildMergeSystem::IsDraggingTower() const
{
    return FindDraggedTowerIndex() >= 0;
}

/*
===============================================================================
Updates drag behaviour.

- While left mouse is held, the dragged tower follows the mouse world position.
- When LMB is released, the tower tries to snap onto the grid.
===============================================================================
*/
void BuildMergeSystem::UpdateDragging(float worldX, float worldY,
    bool lmbDown, bool justReleasedLmb,
    int mouseX, int mouseY)
{
    if (!activeTowers)
        return;

    // While dragging, continuously move tower with mouse
    if (lmbDown)
    {
        TowerHandler::DragAndDropOnce(worldX, worldY, *activeTowers);
    }

    // When player releases mouse, try to snap dragged tower to valid cell
    if (justReleasedLmb && IsDraggingTower())
    {
        SnapDraggedTowerToGrid(mouseX, mouseY);
    }
}

/*
=============================================   ==================================
Rebuilds the occupancy array entirely from the current tower positions.

Every tower's world position is converted into a grid cell, then that cell
is marked occupied.
===============================================================================
*/
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

/*
===============================================================================
Converts a tower's world position into the grid cell it is currently occupying.

This is done by converting world coordinates back into screen space first,
then asking the grid system to convert screen -> grid.
===============================================================================
*/
bool BuildMergeSystem::GetTowerCell(const TowerHandler::Tower& t, GridSystem::GridCoord& outCell) const
{
    if (!grid) return false;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    int mouseX = (int)(t.x + screenW * 0.5f);
    int mouseY = (int)(screenH * 0.5f - t.y);

    return grid->ScreenToGrid(mouseX, mouseY, outCell);
}

/*
===============================================================================
Searches for a tower at a specific grid cell and returns its index.

Returns -1 if no tower exists at that cell.
===============================================================================
*/
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

/*
===============================================================================
Checks whether a tower exists at the specified cell AND whether it matches
the requested type and level.

Used for merge checking.
===============================================================================
*/
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

/*
===============================================================================
Finds all towers connected to the starting cell using BFS-style expansion. 
BFS = Breadth-First Search, a common graph traversal algorithm.

Rules:
- Start from placed tower
- Only include towers with the same type and same level
- 4-directional adjacency only:
    left, right, up, down

outCells will contain the full connected cluster.
===============================================================================
*/
bool BuildMergeSystem::FindConnectedCluster(
    int startX,
    int startY,
    TowerHandler::TowerType type,
    int towerLevel,
    std::vector<GridSystem::GridCoord>& outCells)
{
    outCells.clear();

    if (!level || !activeTowers)
        return false;

    // Start cell itself must match first
    if (!TowerMatchesAtCell(startX, startY, type, towerLevel))
        return false;

    // queue = cells waiting to be explored
    // visited = cells already processed
    std::vector<GridSystem::GridCoord> queue;
    std::vector<GridSystem::GridCoord> visited;

    queue.push_back({ startX, startY });
    size_t front = 0;

    while (front < queue.size())
    {
        GridSystem::GridCoord current = queue[front++];
        bool alreadyVisited = false;

        // Skip cells we already processed before
        for (const auto& v : visited)
        {
            if (v.x == current.x && v.y == current.y)
            {
                alreadyVisited = true;
                break;
            }
        }

        if (alreadyVisited)
            continue;

        visited.push_back(current);

        // If current cell doesn't match type/level, do not expand from it
        if (!TowerMatchesAtCell(current.x, current.y, type, towerLevel))
            continue;

        // This cell is part of the merge cluster
        outCells.push_back(current);

        // Add its 4 neighbors to the BFS queue
        queue.push_back({ current.x + 1, current.y });
        queue.push_back({ current.x - 1, current.y });
        queue.push_back({ current.x, current.y + 1 });
        queue.push_back({ current.x, current.y - 1 });
    }

    return !outCells.empty();
}

/*
===============================================================================
Attempts to merge a tower placed at (placedX, placedY).

New logic:
- Find all connected towers of same type + same level using BFS
- If connected cluster size is 3 or more:
    -> level up placed tower
    -> remove all other towers in the cluster
===============================================================================
*/
bool BuildMergeSystem::TryMergeAtCell(int placedX, int placedY)
{
    if (!activeTowers)
        return false;

    int placedIndex = FindTowerIndexAtCell(placedX, placedY);
    if (placedIndex < 0)
        return false;

    TowerHandler::Tower& placedTower = (*activeTowers)[(size_t)placedIndex];
    TowerHandler::TowerType type = placedTower.details.towerType;
    int towerLevel = placedTower.details.level;

    if (type == TowerHandler::BASE_TOWER)
        return false;

    // Find full connected cluster of same type + same level
    std::vector<GridSystem::GridCoord> cluster;
    if (!FindConnectedCluster(placedX, placedY, type, towerLevel, cluster))
        return false;

    if (cluster.size() < 3)
        return false;

    // Build candidate list excluding the placed tower
    struct Candidate
    {
        GridSystem::GridCoord cell;
        int distSq = 0;
        int priority = 0;
    };

    std::vector<Candidate> candidates;
    for (const auto& c : cluster)
    {
        if (c.x == placedX && c.y == placedY)
            continue;

        int dx = c.x - placedX;
        int dy = c.y - placedY;

        Candidate cand;
        cand.cell = c;
        cand.distSq = dx * dx + dy * dy;

        // Fixed tie-break priority for predictability:
        // up, right, down, left, then diagonals / others by row/col fallback
        if (dx == 0 && dy == -1) cand.priority = 0;      // up
        else if (dx == 1 && dy == 0) cand.priority = 1;  // right
        else if (dx == 0 && dy == 1) cand.priority = 2;  // down
        else if (dx == -1 && dy == 0) cand.priority = 3; // left
        else if (dx == 1 && dy == -1) cand.priority = 4;
        else if (dx == 1 && dy == 1) cand.priority = 5;
        else if (dx == -1 && dy == 1) cand.priority = 6;
        else if (dx == -1 && dy == -1) cand.priority = 7;
        else cand.priority = 100 + (c.y * 100) + c.x;

        candidates.push_back(cand);
    }

    if (candidates.size() < 2)
        return false;

    std::sort(candidates.begin(), candidates.end(),
        [](const Candidate& a, const Candidate& b)
        {
            if (a.distSq != b.distSq)
                return a.distSq < b.distSq;
            return a.priority < b.priority;
        });

    // Pick exactly 2 nearest neighbors + placed tower
    std::vector<int> toRemove;
    for (int i = 0; i < 2; ++i)
    {
        int idx = FindTowerIndexAtCell(candidates[(size_t)i].cell.x, candidates[(size_t)i].cell.y);
        if (idx >= 0)
            toRemove.push_back(idx);
    }

    if (toRemove.size() != 2)
        return false;

    // Upgrade the placed tower only
    (*activeTowers)[(size_t)placedIndex].LevelUp();

    // Remove in descending order so indices remain valid
    std::sort(toRemove.begin(), toRemove.end());
    for (int i = (int)toRemove.size() - 1; i >= 0; --i)
    {
        int idx = toRemove[(size_t)i];

        if (idx < placedIndex)
            --placedIndex;

        RemoveTowerAtIndex(idx);
    }

    return true;
}

/*
===============================================================================
Removes a tower from the active tower list.

Also:
- destroys its sprite
- calls its cleanup/destroy logic
- clears all active bullets as a safety reset
===============================================================================
*/
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

    // Clear bullets to avoid bullets referencing removed towers
    if (activeBullets)
        activeBullets->clear();
}

/*
===============================================================================
Snaps the dragged tower onto the grid when the player releases the mouse.

Cases:
1) mouse is outside grid / out of bounds
   -> return tower to shop slot or destroy if slot not found
2) target tile is not placeable
   -> return tower to shop slot or destroy if slot not found
3) valid tile
   -> snap to tile center
   -> rebuild occupancy
   -> attempt chain merges
===============================================================================
*/
bool BuildMergeSystem::SnapDraggedTowerToGrid(int mouseX, int mouseY)
{
    if (!grid || !shop || !activeTowers)
        return false;

    int draggedIndex = FindDraggedTowerIndex();
    if (draggedIndex < 0)
        return false;

    GridSystem::GridCoord c;

    // Invalid drop location: outside grid or outside level bounds
    if (!grid->ScreenToGrid(mouseX, mouseY, c) || !InBounds(c.x, c.y))
    {
        float slotX = 0.f, slotY = 0.f;
        int slotIdx = (*activeTowers)[(size_t)draggedIndex].sourceSlotIndex;

        // Return dragged tower back to its shop slot if possible
        if (shop->GetSlotCenter(slotIdx, slotX, slotY))
        {
            (*activeTowers)[(size_t)draggedIndex].isDragging = false;
            (*activeTowers)[(size_t)draggedIndex].isReturning = true;
            (*activeTowers)[(size_t)draggedIndex].returnTargetX = slotX;
            (*activeTowers)[(size_t)draggedIndex].returnTargetY = slotY;
        }
        else
        {
            // If slot can't be found, remove tower completely
            RemoveTowerAtIndex(draggedIndex);
            RebuildOccupiedFromTowers();
        }

        return false;
    }

    // Invalid drop location: tile is blocked or occupied
    if (!IsPlaceable(c.x, c.y))
    {
        float slotX = 0.f, slotY = 0.f;
        int slotIdx = (*activeTowers)[(size_t)draggedIndex].sourceSlotIndex;

        // Return to shop slot if available
        if (shop->GetSlotCenter(slotIdx, slotX, slotY))
        {
            (*activeTowers)[(size_t)draggedIndex].isDragging = false;
            (*activeTowers)[(size_t)draggedIndex].isReturning = true;
            (*activeTowers)[(size_t)draggedIndex].returnTargetX = slotX;
            (*activeTowers)[(size_t)draggedIndex].returnTargetY = slotY;
        }
        else
        {
            // Otherwise destroy tower
            RemoveTowerAtIndex(draggedIndex);
            RebuildOccupiedFromTowers();
        }

        return false;
    }

    // Valid placement: snap tower to exact center of grid cell
    float wx = 0.f, wy = 0.f, tileSize = 0.f;
    grid->GetCellWorldCenter({ c.x, c.y }, wx, wy, tileSize);

    (*activeTowers)[(size_t)draggedIndex].x = wx;
    (*activeTowers)[(size_t)draggedIndex].y = wy;
    (*activeTowers)[(size_t)draggedIndex].isDragging = false;
    (*activeTowers)[(size_t)draggedIndex].isSelected = false;
    (*activeTowers)[draggedIndex].isPlaced = true;  // using TS code

    // Update occupancy after placement
    RebuildOccupiedFromTowers();

    // Keep trying to merge repeatedly in case chain merges happen
    while (TryMergeAtCell(c.x, c.y))
    {
        RebuildOccupiedFromTowers();
    }

    RebuildOccupiedFromTowers();
    return true;
}

/*
===============================================================================
Builds a reusable "X" mesh used to mark blocked/unplaceable cells.

The X is made from 2 thin quads crossing each other diagonally.
===============================================================================
*/
void BuildMergeSystem::BuildXMeshIfNeeded()
{
    if (xMesh) return;

    const float half = 0.45f;
    const float thick = 0.06f;

    // Helper lambda: creates a thin quad along a line
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

/*
===============================================================================
Frees the reusable X mesh.
===============================================================================
*/
void BuildMergeSystem::FreeXMesh()
{
    if (xMesh)
    {
        AEGfxMeshFree(xMesh);
        xMesh = nullptr;
    }
}

/*
===============================================================================
Draws an X marker over a single cell.

Used to show cells that cannot currently be placed on.
===============================================================================
*/
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

/*
===============================================================================
Draws the build overlay.

- Draws the grid
- Draws X markers on blocked/unplaceable tiles
- Highlights the tile under the mouse:
    green = placeable
    red   = blocked
===============================================================================
*/
void BuildMergeSystem::DrawOverlay()
{
    if (!grid || !level || !occupied)
        return;

    const size_t expected = (size_t)level->width * (size_t)level->height;
    if (expected == 0 || occupied->size() != expected)
        return;

    // Draw base grid first
    grid->Draw();

    // Draw X markers on every non-placeable tile
    for (int y = 0; y < level->height; ++y)
    {
        for (int x = 0; x < level->width; ++x)
        {
            if (!IsPlaceable(x, y))
                DrawXAtCell(x, y, 0.75f);
        }
    }

    // Highlight tile currently under the mouse cursor
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (grid->ScreenToGrid(mx, my, c))
    {
        if (IsPlaceable(c.x, c.y))
            grid->DrawTileTinted(c, 0.2f, 1.0f, 0.2f, 0.55f); // green
        else
            grid->DrawTileTinted(c, 1.0f, 0.2f, 0.2f, 0.55f); // red
    }
}