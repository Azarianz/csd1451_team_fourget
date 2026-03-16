#include "LevelLoader.h"
#include <fstream>
#include <string>
#include <cstdio>

bool LevelLoader::Init(const char* levelPath)
{
    Shutdown();

    if (!LoadFromText(levelPath))
    {
        PRINT("Loading level file: %s\n", levelPath);
        PRINT("FAILED to load level: %s\n", levelPath);
        return false;
    }

    if (!IsValid())
    {
        const size_t expected = (size_t)width * (size_t)height;
        PRINT("LEVEL DATA SIZE MISMATCH! w=%d h=%d expected=%zu map=%zu region=%zu\n",
            width, height, expected, map.size(), region.size());
        return false;
    }

    return true;
}

bool LevelLoader::IsValid() const
{
    const size_t expected = (size_t)width * (size_t)height;
    return expected > 0 &&
        map.size() == expected &&
        region.size() == expected;
}

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

    return EnsureRenderReady();
}

bool LevelLoader::BuildPath(const GridSystem::Grid& grid, std::vector<Point>& outPath) const
{
    outPath.clear();

    if (width <= 0 || height <= 0)
        return false;

    GridSystem::GridCoord spawn{};
    if (!FindSpawnCell(spawn))
    {
        PRINT("No ENEMYSPAWN found.\n");
        return false;
    }

    PushCellCenterToPath(grid, spawn, outPath);

    GridSystem::GridCoord current = spawn;
    GridSystem::GridCoord next{};

    if (!FindNextFromSpawn(spawn, next))
    {
        PRINT("Spawn does not connect to any enemy path or goal.\n");
        return false;
    }

    const int maxSteps = width * height + 8;

    for (int step = 0; step < maxSteps; ++step)
    {
        current = next;
        PushCellCenterToPath(grid, current, outPath);

        RegionFlag flag = static_cast<RegionFlag>(region[Idx(current.x, current.y)]);

        if (flag == RegionFlag::ENEMYGOAL)
        {
            PRINT("Enemy path built. Total points: %d\n", (int)outPath.size());
            return true;
        }

        if (!StepFromRegionFlag(current, next))
        {
            PRINT("Broken enemy path at cell (%d, %d)\n", current.x, current.y);
            return false;
        }
    }

    PRINT("Enemy path exceeded safety limit. Possible loop.\n");
    return false;
}

bool LevelLoader::FindSpawnCell(GridSystem::GridCoord& outSpawn) const
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            RegionFlag flag = static_cast<RegionFlag>(region[Idx(x, y)]);
            if (flag == RegionFlag::ENEMYSPAWN)
            {
                outSpawn = { x, y };
                return true;
            }
        }
    }

    return false;
}

bool LevelLoader::FindNextFromSpawn(const GridSystem::GridCoord& spawn, GridSystem::GridCoord& outNext) const
{
    const GridSystem::GridCoord neighbors[4] =
    {
        { spawn.x,     spawn.y - 1 },
        { spawn.x,     spawn.y + 1 },
        { spawn.x - 1, spawn.y     },
        { spawn.x + 1, spawn.y     }
    };

    for (const auto& n : neighbors)
    {
        if (!InBounds(n.x, n.y))
            continue;

        RegionFlag flag = static_cast<RegionFlag>(region[Idx(n.x, n.y)]);

        if (flag == RegionFlag::ENEMYGOAL ||
            flag == RegionFlag::ENEMYPATH_UP ||
            flag == RegionFlag::ENEMYPATH_DOWN ||
            flag == RegionFlag::ENEMYPATH_LEFT ||
            flag == RegionFlag::ENEMYPATH_RIGHT)
        {
            outNext = n;
            return true;
        }
    }

    return false;
}

bool LevelLoader::StepFromRegionFlag(const GridSystem::GridCoord& current, GridSystem::GridCoord& outNext) const
{
    RegionFlag flag = static_cast<RegionFlag>(region[Idx(current.x, current.y)]);
    outNext = current;

    switch (flag)
    {
    case RegionFlag::ENEMYPATH_UP:
        outNext.y -= 1;
        break;

    case RegionFlag::ENEMYPATH_DOWN:
        outNext.y += 1;
        break;

    case RegionFlag::ENEMYPATH_LEFT:
        outNext.x -= 1;
        break;

    case RegionFlag::ENEMYPATH_RIGHT:
        outNext.x += 1;
        break;

    default:
        return false;
    }

    if (!InBounds(outNext.x, outNext.y))
        return false;

    return true;
}

void LevelLoader::PushCellCenterToPath(const GridSystem::Grid& grid,
    const GridSystem::GridCoord& cell,
    std::vector<Point>& outPath) const
{
    float wx = 0.0f;
    float wy = 0.0f;
    float tileSize = 0.0f;
    grid.GetCellWorldCenter(cell, wx, wy, tileSize);
    outPath.push_back({ wx, wy });
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