#pragma once
#include <vector>
#include <cstdint>

#include "AEEngine.h"
#include "GridSystem.h"
#include "LevelData.h"
#include "Enemy.h" // for Point

class LevelLoader
{
public:
    int width = 0;
    int height = 0;

    std::vector<int> map;        // tile IDs
    std::vector<uint8_t> region; // region flags

    int Idx(int x, int y) const { return y * width + x; }
    bool InBounds(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }

public:
    bool Init(int levelIndex);
    bool IsValid() const;

    bool LoadFromText(const char* path);
    bool BuildPath(const GridSystem::Grid& grid, std::vector<Point>& outPath) const;

    void Draw(const GridSystem::Grid& grid, float alphaMul = 1.0f) const;
    void Shutdown();

private:
    bool FindSpawnCell(GridSystem::GridCoord& outSpawn) const;
    bool FindNextFromSpawn(const GridSystem::GridCoord& spawn, GridSystem::GridCoord& outNext) const;
    bool StepFromRegionFlag(const GridSystem::GridCoord& current, GridSystem::GridCoord& outNext) const;
    void PushCellCenterToPath(const GridSystem::Grid& grid,
        const GridSystem::GridCoord& cell,
        std::vector<Point>& outPath) const;

private:
    // render cache
    AEGfxTexture* m_tilesetTex = nullptr;
    int m_tilesetCols = 0;
    int m_tilesetRows = 0;
    int m_maxTileId = 0;
    mutable std::vector<AEGfxVertexList*> m_tileMeshes;

    AEGfxVertexList* GetTileMesh(int tileId) const;
    bool EnsureRenderReady();
};