#include <cmath>   // std::floor
#include "AEEngine.h"
#include "GridSystem.h"

namespace GridSystem {
    static float MinF(float a, float b) { return (a < b) ? a : b; }

    float RoundToPixel(float v)
    {
        return floorf(v + 0.5f);
    }

    Grid::Grid(int width, int height, float cellSize, Vec2 originWorld)
        : m_width(width)
        , m_height(height)
        , m_cellSize(cellSize)
        , m_originWorld(originWorld)
        , m_cells()
    {
        if (m_width < 0)  m_width = 0;
        if (m_height < 0) m_height = 0;
        if (m_cellSize <= 0.0f) m_cellSize = 1.0f;

        m_cells.assign(static_cast<std::size_t>(m_width) * static_cast<std::size_t>(m_height), 0);
    }

    std::size_t Grid::Index(int x, int y) const
    {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width)
            + static_cast<std::size_t>(x);
    }

    bool Grid::InBounds(int x, int y) const
    {
        return (x >= 0 && x < m_width && y >= 0 && y < m_height);
    }

    bool Grid::InBounds(GridCoord c) const
    {
        return InBounds(c.x, c.y);
    }

    GridCoord Grid::WorldToGrid(Vec2 world) const
    {
        // translate relative to origin
        const float localX = (world.x - m_originWorld.x) / m_cellSize;
        const float localY = (world.y - m_originWorld.y) / m_cellSize;

        // floor into cell index
        GridCoord c;
        c.x = static_cast<int>(std::floor(localX));
        c.y = static_cast<int>(std::floor(localY));
        return c;
    }

    Vec2 Grid::GridToWorldMin(GridCoord c) const
    {
        // bottom-left corner of tile
        Vec2 w;
        w.x = m_originWorld.x + (static_cast<float>(c.x) * m_cellSize);
        w.y = m_originWorld.y + (static_cast<float>(c.y) * m_cellSize);
        return w;
    }

    Vec2 Grid::GridToWorldCenter(GridCoord c) const
    {
        Vec2 min = GridToWorldMin(c);
        Vec2 center;
        center.x = min.x + m_cellSize * 0.5f;
        center.y = min.y + m_cellSize * 0.5f;
        return center;
    }

    bool Grid::Place(GridCoord c, int objectId)
    {
        if (!InBounds(c)) return false;
        if (objectId == 0) objectId = 1; // keep 0 as "empty"

        const std::size_t idx = Index(c.x, c.y);
        if (m_cells[idx] != 0) return false; // already occupied

        m_cells[idx] = objectId;
        return true;
    }

    bool Grid::Remove(GridCoord c)
    {
        if (!InBounds(c)) return false;

        const std::size_t idx = Index(c.x, c.y);
        if (m_cells[idx] == 0) return false; // already empty

        m_cells[idx] = 0;
        return true;
    }

    bool Grid::IsOccupied(GridCoord c) const
    {
        if (!InBounds(c)) return false;
        return m_cells[Index(c.x, c.y)] != 0;
    }

    int Grid::GetObjectId(GridCoord c) const
    {
        if (!InBounds(c)) return 0;
        return m_cells[Index(c.x, c.y)];
    }

    void Grid::Clear()
    {
        for (int& v : m_cells)
            v = 0;
    }

    void Grid::InitScene() {
        // Your tile sprite (single square tile image)
        pTileTex = AEGfxTextureLoad("Assets/grid_highlighted.png");
        if (!pTileTex)
        {
            // If this prints in your debugger output, your texture isn't loading.
            // Common cause: AE build doesn't support PNG, or wrong relative path.
            OutputDebugStringA("FAILED to load Assets/grid_highlighted.png\n");
        }

        // Build a unit quad mesh (VertexList) with UVs; we scale per-tile
        pTileMesh = nullptr;

        AEGfxMeshStart();

        // Centered quad (-0.5..0.5) with UVs
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

        pTileMesh = AEGfxMeshEnd();
    }

    void Grid::Draw() {
        // Toggle grid visibility
        if (AEInputCheckTriggered(AEVK_G))
            showGrid = !showGrid;

        // Render setup
        AEGfxSetBackgroundColor(0.25f, 0.25f, 0.25f);
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxTextureSet(pTileTex, 0, 0);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1, 1, 1, 1);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        // -----------------------------
        // DRAW GRID (auto-fit to screen)
        // -----------------------------
        //if (showGrid && pTileTex && pTileMesh)
                //if (showGrid && pTileTex && pTileMesh)
        if (showGrid && pTileTex && pTileMesh)
        {
            // Fit inside screen (no cropping)
            float screenW = (float)AEGfxGetWindowWidth();
            float screenH = (float)AEGfxGetWindowHeight();
            float tileSizeW = screenW / (float)cols;
            float tileSizeH = screenH / (float)rows;
            float tileSize = MinF(tileSizeW, tileSizeH);

            // Pixel-art friendliness: snap to integer pixels
            tileSize = floorf(tileSize);
            if (tileSize < 1.0f) tileSize = 1.0f;

            float gridW = tileSize * (float)cols;
            float gridH = tileSize * (float)rows;

            // Center the grid
            float startX = -gridW * 0.5f + tileSize * 0.5f;
            float startY = gridH * 0.5f - tileSize * 0.5f;

            for (int y = 0; y < rows; ++y)
            {
                for (int x = 0; x < cols; ++x)
                {
                    float worldX = startX + (float)x * tileSize;
                    float worldY = startY - (float)y * tileSize;

                    // Snap to pixel to reduce shimmer
                    worldX = GridSystem::RoundToPixel(worldX);
                    worldY = GridSystem::RoundToPixel(worldY);

                    AEMtx33 scale, trans, m;
                    AEMtx33Scale(&scale, tileSize, tileSize);
                    AEMtx33Trans(&trans, worldX, worldY);
                    AEMtx33Concat(&m, &trans, &scale);

                    AEGfxSetTransform(m.m);
                    AEGfxMeshDraw(pTileMesh, AE_GFX_MDM_TRIANGLES);
                }
            }
        }
    }

    void Grid::Update()
    {
        Draw();
        //Logic
	}

    void Grid::Destroy() {
        if (pTileMesh) AEGfxMeshFree(pTileMesh);
        if (pTileTex)  AEGfxTextureUnload(pTileTex);
    }
}