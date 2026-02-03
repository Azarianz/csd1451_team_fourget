#pragma once
#include <vector>
#include <cstdint>

#include "AEEngine.h"
#include "GridSystem.h"

class LevelLoader
{
public:
    int width = 0;
    int height = 0;

    std::vector<int> map;        // tile IDs
    std::vector<uint8_t> region; // 0 none, 1 buildable, 2 path

public:
    bool LoadFromText(const char* path);

    void Draw(const GridSystem::Grid& grid, float alphaMul = 1.0f) const;
    void Shutdown();

private:
    int Idx(int x, int y) const { return y * width + x; }

    // render cache (same idea as your editor)
    AEGfxTexture* m_tilesetTex = nullptr;
    int m_tilesetCols = 0;
    int m_tilesetRows = 0;
    int m_maxTileId = 0;
    mutable std::vector<AEGfxVertexList*> m_tileMeshes;

    AEGfxVertexList* GetTileMesh(int tileId) const;
    bool EnsureRenderReady(); // loads tilesheet if not loaded yet
};