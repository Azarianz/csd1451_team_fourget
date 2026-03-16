#pragma once

#include "AEEngine.h"
#include "GridSystem.h"
#include "LevelLoader.h"
#include "Shop.h"
#include "Tower.h"

#include <vector>
#include <cstdint>

class BuildMergeSystem
{
public:
    BuildMergeSystem() = default;
    ~BuildMergeSystem() = default;

    void Init(LevelLoader* levelPtr,
        GridSystem::Grid* gridPtr,
        TowerHandler::Shop* shopPtr,
        std::vector<TowerHandler::Tower>* towersPtr,
        std::vector<TowerHandler::ActiveBullet>* bulletsPtr,
        std::vector<uint8_t>* occupiedPtr);

    void Shutdown();

    void UpdateDragging(float worldX, float worldY,
        bool lmbDown, bool justReleasedLmb,
        int mouseX, int mouseY);

    void DrawOverlay();

    bool IsDraggingTower() const;
    int  FindDraggedTowerIndex() const;

    void RebuildOccupiedFromTowers();
    bool SnapDraggedTowerToGrid(int mouseX, int mouseY);

    bool GetTowerCell(const TowerHandler::Tower& t, GridSystem::GridCoord& outCell) const;
    int  FindTowerIndexAtCell(int x, int y) const;
    bool TowerMatchesAtCell(int x, int y, TowerHandler::TowerType type, int towerLevel) const;
    bool TryMergeAtCell(int placedX, int placedY);
    void RemoveTowerAtIndex(int idx);

    bool FindConnectedCluster(int startX, int startY,
        TowerHandler::TowerType type, int towerLevel,
        std::vector<GridSystem::GridCoord>& outCells);
private:
    LevelLoader* level = nullptr;
    GridSystem::Grid* grid = nullptr;
    TowerHandler::Shop* shop = nullptr;
    std::vector<TowerHandler::Tower>* activeTowers = nullptr;
    std::vector<TowerHandler::ActiveBullet>* activeBullets = nullptr;
    std::vector<uint8_t>* occupied = nullptr;

    AEGfxVertexList* xMesh = nullptr;

    int  Idx(int x, int y) const;
    bool InBounds(int x, int y) const;
    bool IsBuildable(int x, int y) const;
    bool IsOccupied(int x, int y) const;
    bool IsPlaceable(int x, int y) const;

    void BuildXMeshIfNeeded();
    void FreeXMesh();
    void DrawXAtCell(int x, int y, float alpha);
};
