#pragma once
#include <vector>
#include <cstddef>
#include <cmath>
#include "AEEngine.h"

namespace GridSystem {

    float RoundToPixel(float v);

    struct GridCoord
    {
        int x{ 0 };
        int y{ 0 };
        bool operator==(const GridCoord& rhs) const { return x == rhs.x && y == rhs.y; }
        bool operator!=(const GridCoord& rhs) const { return !(*this == rhs); }
    };

    struct Vec2 { float x{ 0 }, y{ 0 }; };

    class Grid
    {
    public:
        Grid(int width, int height, float cellSize, Vec2 originWorld = { 0.0f, 0.0f });

        AEGfxTexture* pTileTex = nullptr;
        AEGfxVertexList* pTileMesh = nullptr;

        bool showGrid = true;

        int   GetWidth()  const { return m_width; }
        int   GetHeight() const { return m_height; }

        bool InBounds(GridCoord c) const;
        bool InBounds(int x, int y) const;

        void Init();
        void Draw();     // base grid draw (kept)
        void Destroy();

        // -----------------------------
        // EDITOR HELPERS (NEW)
        // -----------------------------
        // Convert screen mouse (pixels) -> grid coord based on your current draw layout.
        // Returns false if mouse not over grid.
        bool ScreenToGrid(int mouseX, int mouseY, GridCoord& out) const;

        // Draw a tinted tile at a given grid coord (uses same layout as Draw()).
        void DrawTileTinted(GridCoord c, float r, float g, float b, float a) const;

    private:
        int m_width{ 0 };
        int m_height{ 0 };
        float m_cellSize{ 1.0f };
        Vec2 m_originWorld{ 0.0f, 0.0f };

        std::vector<int> m_cells;

        std::size_t Index(int x, int y) const;

        // layout helper (same math as Draw())
        void ComputeLayout(float& tileW, float& tileH, float& startX, float& startY) const;
    };
}