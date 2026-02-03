#pragma once
#include "Scene.h"
#include "GridSystem.h"
#include "LevelLoader.h"
#include <vector>

//This scene will take an int ID as parameter to load different levels
class Scene_LoadLevel : public Scene
{
public:
    // SceneManager will call this before Init()
    void SetLevelIndex(int idx) { levelIndex = idx; }

    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    LevelLoader level;
    GridSystem::Grid* grid = nullptr;

    int levelIndex = 1;                 // default
    char levelPath[128] = { 0 };        // built from index

    bool LoadLevel(int idx);            // helper (rebuilds grid, occupied, etc.)

    // ---- Build Mode ----
    bool buildMode = false;
    std::vector<uint8_t> occupied; // runtime tower occupancy (0/1)

    // X marker rendering
    AEGfxVertexList* xMesh = nullptr;

private:
    int  Idx(int x, int y) const { return level.Idx(x, y); }
    bool IsBuildable(int x, int y) const { return level.region[Idx(x, y)] == 1; }
    bool IsOccupied(int x, int y)  const { return occupied[Idx(x, y)] != 0; }
    bool IsPlaceable(int x, int y) const { return IsBuildable(x, y) && !IsOccupied(x, y); }

    void BuildXMeshIfNeeded();
    void DrawXAtCell(int x, int y, float alpha);
    void DrawBuildOverlay();
};