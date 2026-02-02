#include "GridSystem.h"

namespace GridSystem {

    static float MinF(float a, float b) { return (a < b) ? a : b; }

    float RoundToPixel(float v) { return floorf(v + 0.5f); }

    Grid::Grid(int width, int height, float cellSize, Vec2 originWorld)
        : m_width(width), m_height(height), m_cellSize(cellSize), m_originWorld(originWorld), m_cells()
    {
        if (m_width < 0)  m_width = 0;
        if (m_height < 0) m_height = 0;
        if (m_cellSize <= 0.0f) m_cellSize = 1.0f;

        m_cells.assign((size_t)m_width * (size_t)m_height, 0);
    }

    std::size_t Grid::Index(int x, int y) const
    {
        return (size_t)y * (size_t)m_width + (size_t)x;
    }

    bool Grid::InBounds(int x, int y) const
    {
        return (x >= 0 && x < m_width && y >= 0 && y < m_height);
    }

    bool Grid::InBounds(GridCoord c) const { return InBounds(c.x, c.y); }

    void Grid::Init()
    {
        pTileMesh = nullptr;
        pTileTex = nullptr;

        pTileTex = AEGfxTextureLoad("Assets/grid_highlighted.png");
        if (!pTileTex) PRINT("FAILED to load Assets/grid_highlighted.png\n");
        else {
            PRINT("SUCCESSFULLY loaded Assets/grid_highlighted.png\n");
        }

        AEGfxMeshStart();

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

        pTileMesh = AEGfxMeshEnd();
        if (!pTileMesh) PRINT("Grid Init: pTileMesh is NULL\n");
        else {
            PRINT("SUCCESSFULLY init pTileMesh\n");
        }
    } 

    void Grid::ComputeLayout(float& tileW, float& tileH, float& startX, float& startY) const
    {
        float screenW = (float)AEGfxGetWindowWidth();
        float screenH = (float)AEGfxGetWindowHeight();

        tileW = screenW / (float)m_width;
        tileH = screenH / (float)m_height;

        // center of cell (0,0) top-left-ish
        startX = -screenW * 0.5f + tileW * 0.5f;
        startY = screenH * 0.5f - tileH * 0.5f;
    }

    void Grid::Draw()
    {
        if (AEInputCheckTriggered(AEVK_G))
            showGrid = !showGrid;

        if (!showGrid || !pTileMesh) return;
        if (m_width <= 0 || m_height <= 0) return;
        if (!pTileTex) return;

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(pTileTex, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

        float tileW, tileH, startX, startY;
        ComputeLayout(tileW, tileH, startX, startY);

        // 1) Base grid faint
        const float baseAlpha = 0.12f;
        AEGfxSetTransparency(baseAlpha);
        AEGfxSetColorToMultiply(1, 1, 1, 1);
        AEGfxSetColorToAdd(0, 0, 0, 0);

        for (int y = 0; y < m_height; ++y)
        {
            for (int x = 0; x < m_width; ++x)
            {
                float worldX = startX + (float)x * tileW;
                float worldY = startY - (float)y * tileH;

                worldX = RoundToPixel(worldX);
                worldY = RoundToPixel(worldY);

                AEMtx33 scale, trans, m;
                AEMtx33Scale(&scale, tileW, tileH);
                AEMtx33Trans(&trans, worldX, worldY);
                AEMtx33Concat(&m, &trans, &scale);

                AEGfxSetTransform(m.m);
                AEGfxMeshDraw(pTileMesh, AE_GFX_MDM_TRIANGLES);
            }
        }

        // 2) Hover highlight bright (opacity-only)
        int mx = 0, my = 0;
        AEInputGetCursorPosition(&mx, &my);

        GridCoord hover;
        if (ScreenToGrid(mx, my, hover))
        {
            // reuse your existing function
            DrawTileTinted(hover, 1, 1, 1, 1.0f);
        }

        // Reset
        AEGfxSetTransparency(1.0f);
    }

    void Grid::DrawTileTinted(GridCoord c, float r, float g, float b, float a) const
    {
        if (!pTileMesh) return;
        if (!InBounds(c)) return;
        if (!pTileTex) return;


        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(pTileTex, 0, 0);

        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(a);                 // opacity lives here
        AEGfxSetColorToMultiply(r, g, b, 1.0f);  // no alpha here
        AEGfxSetColorToAdd(0, 0, 0, 0);

        float tileW, tileH, startX, startY;
        ComputeLayout(tileW, tileH, startX, startY);

        float worldX = startX + (float)c.x * tileW;
        float worldY = startY - (float)c.y * tileH;

        worldX = RoundToPixel(worldX);
        worldY = RoundToPixel(worldY);

        AEMtx33 scale, trans, m;
        AEMtx33Scale(&scale, tileW, tileH);
        AEMtx33Trans(&trans, worldX, worldY);
        AEMtx33Concat(&m, &trans, &scale);

        AEGfxSetTransform(m.m);
        AEGfxMeshDraw(pTileMesh, AE_GFX_MDM_TRIANGLES);

        AEGfxSetTransparency(1.0f);
    }

    bool Grid::ScreenToGrid(int mouseX, int mouseY, GridCoord& out) const
    {
        if (m_width <= 0 || m_height <= 0) return false;

        float screenW = (float)AEGfxGetWindowWidth();
        float screenH = (float)AEGfxGetWindowHeight();

        // screen (0,0) top-left -> world (0,0) center, y up
        float worldX = (float)mouseX - screenW * 0.5f;
        float worldY = screenH * 0.5f - (float)mouseY;

        float tileW, tileH, startX, startY;
        ComputeLayout(tileW, tileH, startX, startY);

        // convert to grid indices (x increases right, y increases down)
        float localX = (worldX - (startX - tileW * 0.5f)) / tileW;
        float localY = ((startY + tileH * 0.5f) - worldY) / tileH;

        int gx = (int)floorf(localX);
        int gy = (int)floorf(localY);

        if (!InBounds(gx, gy)) return false;

        out.x = gx;
        out.y = gy;
        return true;
    }

    void Grid::Destroy()
    {
        if (pTileMesh) AEGfxMeshFree(pTileMesh);
        if (pTileTex)  AEGfxTextureUnload(pTileTex);
        pTileMesh = nullptr;
        pTileTex = nullptr;
    }
}