#include "LevelLoader.h"
#include <fstream>
#include <string>

static bool ReadToken(std::istream& in, std::string& out)
{
    return (in >> out) ? true : false;
}

bool LevelLoader::LoadFromText(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open())
        return false;

    std::string tok;

    // Magic
    if (!ReadToken(file, tok)) return false;
    if (tok != "MERGE_DEFENDERS_LEVEL_V1")
        return false;

    // width height
    file >> width >> height;
    if (width <= 0 || height <= 0) return false;

    map.assign((size_t)width * height, 0);
    region.assign((size_t)width * height, 0);

    // Expect MAP
    if (!ReadToken(file, tok)) return false;
    if (tok != "MAP") return false;

    // Read map numbers
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            file >> map[Idx(x, y)];

    // Expect REGION
    if (!ReadToken(file, tok)) return false;
    if (tok != "REGION") return false;

    // Read region numbers
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int v = 0;
            file >> v;
            if (v < 0) v = 0;
            if (v > 255) v = 255;
            region[Idx(x, y)] = (uint8_t)v;
        }
    }

    // Prepare render cache (tilesheet + mesh cache)
    return EnsureRenderReady();
}

bool LevelLoader::EnsureRenderReady()
{
    if (m_tilesetTex)
        return true;

    m_tilesetTex = AEGfxTextureLoad("Assets/tilesheet.png");
    if (!m_tilesetTex)
    {
        PRINT("FAILED to load tileset: Assets/tilesheet.png\n");
        return false;
    }

    // Must match your tilesheet assumptions
    const int tilePx = 16;
    const int texW = 80;
    const int texH = 160;

    m_tilesetCols = texW / tilePx;
    m_tilesetRows = texH / tilePx;
    m_maxTileId = m_tilesetCols * m_tilesetRows;

    m_tileMeshes.assign((size_t)m_maxTileId + 1, nullptr);
    return true;
}

void LevelLoader::Shutdown()
{
    for (auto& m : m_tileMeshes)
    {
        if (m) AEGfxMeshFree(m);
        m = nullptr;
    }
    m_tileMeshes.clear();

    if (m_tilesetTex)
    {
        AEGfxTextureUnload(m_tilesetTex);
        m_tilesetTex = nullptr;
    }
}

AEGfxVertexList* LevelLoader::GetTileMesh(int tileId) const
{
    if (tileId <= 0 || tileId > m_maxTileId) return nullptr;
    if ((int)m_tileMeshes.size() <= tileId) return nullptr;

    if (m_tileMeshes[tileId]) return m_tileMeshes[tileId];

    int index = tileId - 1;
    int tx = index % m_tilesetCols;
    int ty = index / m_tilesetCols;

    // Flip so row 0 == TOP row of texture
    ty = (m_tilesetRows - 1) - ty;
    if (ty < 0 || ty >= m_tilesetRows) return nullptr;

    float u0 = (float)tx / (float)m_tilesetCols;
    float u1 = (float)(tx + 1) / (float)m_tilesetCols;
    float v0 = (float)ty / (float)m_tilesetRows;
    float v1 = (float)(ty + 1) / (float)m_tilesetRows;

    AEGfxMeshStart();

    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, -0.5f, 0xFFFFFFFF, u1, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0
    );

    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0,
        -0.5f, 0.5f, 0xFFFFFFFF, u0, v0
    );

    m_tileMeshes[tileId] = AEGfxMeshEnd();
    return m_tileMeshes[tileId];
}

void LevelLoader::Draw(const GridSystem::Grid& grid, float alphaMul) const
{
    if (!m_tilesetTex) return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_tilesetTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0, 0, 0, 0);
    AEGfxSetColorToMultiply(1, 1, 1, 1);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            int id = map[Idx(x, y)];
            if (id == 0) continue;

            AEGfxVertexList* mesh = GetTileMesh(id);
            if (!mesh) continue;

            float wx, wy, tileSize;
            grid.GetCellWorldCenter({ x, y }, wx, wy, tileSize);

            AEGfxSetTransparency(alphaMul);

            AEMtx33 scale, trans, m;
            AEMtx33Scale(&scale, tileSize, tileSize);
            AEMtx33Trans(&trans, wx, wy);
            AEMtx33Concat(&m, &trans, &scale);

            AEGfxSetTransform(m.m);
            AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    AEGfxSetTransparency(1.0f);
}