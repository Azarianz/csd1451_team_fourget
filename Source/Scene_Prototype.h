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

    // --- Enemies ---
    std::vector<Point> path;
    std::vector<Enemy> enemies;
    float spawnTimer = 0.0f;
    float spawnInterval = 1.0f;

    // --- Shop + Towers ---
    TowerHandler::Shop shop;
    std::vector<TowerHandler::Tower> activeTowers;

    // runtime occupancy (for build placement)
    std::vector<uint8_t> occupied; // 0/1

    // ----- Build Overlay -----
    bool buildMode = false;
    bool wasLmbDown = false;
    AEGfxVertexList* xMesh = nullptr;

private:
    // drag helpers
    int  FindDraggedTowerIndex() const;
    bool IsDraggingTower() const;

    // level helpers
    bool LoadLevel(int idx);
    void BuildPathFromRegionScan();

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