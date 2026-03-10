#pragma once
#include "Scene.h"
#include "GridSystem.h"
#include "LevelLoader.h"
#include "Enemy.h"
#include "Shop.h"
#include "Tower.h"

#include <vector>
#include <cstdint>

class Scene_Prototype : public Scene
{
public:
    void SetLevelIndex(int idx) { levelIndex = idx; }

    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    // ----- Level -----
    int levelIndex = 1;
    char levelPath[128] = { 0 };
    LevelLoader level;
    GridSystem::Grid* grid = nullptr;

    int baseTowerIndex = -1;
    bool gameOver = false;
    s8 gameOverFont = -1;

    // --- Enemies ---
    WaveManager waveManager;
    s8 m_uiFont = -1;
    void DrawUI();

    std::vector<Point> path;
    std::vector<Enemy*> enemies;

    // --- Shop + Towers ---
    TowerHandler::Shop shop;
    std::vector<TowerHandler::Tower> activeTowers;
    std::vector<TowerHandler::ActiveBullet> activeBullets;

    // runtime occupancy (for build placement)
    std::vector<uint8_t> occupied; // 0/1

    // ----- Build Overlay -----
    bool buildMode = false;
    bool wasLmbDown = false;
    AEGfxVertexList* xMesh = nullptr;

    // drag helpers
    int  FindDraggedTowerIndex() const;
    bool IsDraggingTower() const;

    // selection helpers
    void ClearTowerSelection();
    int  FindPlacedTowerAtMouse(float worldX, float worldY) const;
    void HandleTowerSelection(float worldX, float worldY, bool justPressedLmb);

    // merge helpers
    void RebuildOccupiedFromTowers();
    bool GetTowerCell(const TowerHandler::Tower& t, GridSystem::GridCoord& outCell) const;
    int  FindTowerIndexAtCell(int x, int y) const;
    bool TowerMatchesAtCell(int x, int y, TowerHandler::TowerType type, int towerLevel) const;
    bool TryMergeAtCell(int placedX, int placedY);
    void RemoveTowerAtIndex(int idx);

    // level helpers
    bool LoadLevel(int idx);
    bool BuildPathFromRegionScan();
    bool FindSpawnCell(GridSystem::GridCoord& outSpawn) const;
    bool FindNextFromSpawn(const GridSystem::GridCoord& spawn, GridSystem::GridCoord& outNext) const;
    bool StepFromRegionFlag(const GridSystem::GridCoord& current, GridSystem::GridCoord& outNext) const;
    void PushCellCenterToPath(const GridSystem::GridCoord& cell);

    // build overlay render
    void BuildXMeshIfNeeded();
    void DrawBuildOverlay();
    void DrawXAtCell(int x, int y, float alpha);

    // placement
    void SnapDraggedTowerToGrid(int mouseX, int mouseY);

    // placement helpers
    int  Idx(int x, int y) const { return level.Idx(x, y); }
    bool InBounds(int x, int y) const { return x >= 0 && y >= 0 && x < level.width && y < level.height; }

    bool IsBuildable(int x, int y) const
    {
        int i = Idx(x, y);
        if (i < 0) return false;
        if ((size_t)i >= level.region.size()) return false;
        return level.region[i] == 1; // 1 buildable
    }
    bool IsOccupied(int x, int y) const
    {
        int i = Idx(x, y);
        if (i < 0) return false;
        if ((size_t)i >= occupied.size()) return false;
        return occupied[i] != 0;
    }
    bool IsPlaceable(int x, int y) const
    {
        return IsBuildable(x, y) && !IsOccupied(x, y);
    }
};