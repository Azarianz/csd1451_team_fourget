#pragma once
#include "GridSystem.h"
#include "LevelData.h"
#include <string>
#include <vector>

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

    bool m_showUI = true;

    ActiveLayer m_layer = ActiveLayer::MapLayer;

    int m_currentTileId = 1;
    RegionFlag m_currentRegion = RegionFlag::BUILDABLE;

    // Tileset rendering
    AEGfxTexture* m_tilesetTex = nullptr;
    int m_tilesetCols = 1;
    int m_tilesetRows = 1;
    int m_maxTileId = 9; // you clamp brush 0..9
    std::vector<AEGfxVertexList*> m_tileMeshes; // 0..m_maxTileId

    void HandleFileNameTyping();
    std::string saveFileName = "level_01";
    bool m_isTypingFileName = false;
    std::string m_fileNameBeforeTyping;

    void PaintAtMouse(bool erase);
    void DrawRegionOverlay() const;
    void DrawMapOverlay() const;
    void DrawUI() const;

    void CycleMapTile(int dir);
    void CycleRegion(int dir);

    AEGfxVertexList* GetTileMesh(int tileId);
    void DrawMapTiles(float alphaMul) const;
    void DrawRegionNumbers() const;
    void DrawBrushPreview(float px, float py, float sizePx) const;
};