#pragma once
#include "GridSystem.h"
#include "LevelData.h"
#include <string>

class LevelEditor
{
public:
    s8 m_uiFont = -1;
    enum class ActiveLayer
    {
        MapLayer,
        RegionLayer
    };

    void Init(int w, int h);
    void Update(float dt);
    void Draw();
    void Shutdown();

    LevelData& GetData() { return m_level; }

private:
    GridSystem::Grid* m_grid = nullptr;
    LevelData m_level;

    ActiveLayer m_layer = ActiveLayer::MapLayer;

    int m_currentTileId = 1;
    RegionFlag m_currentRegion = RegionFlag::BUILDABLE;

    std::string m_path = "level_01.txt";

    void PaintAtMouse(bool erase);
    void DrawRegionOverlay() const;
    void DrawMapOverlay() const;
    void DrawUI() const;

    void CycleMapTile(int dir);
    void CycleRegion(int dir);
};