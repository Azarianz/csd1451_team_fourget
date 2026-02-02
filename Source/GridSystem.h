#pragma once
#include <vector>
#include <cstddef>

namespace GridSystem {
    float RoundToPixel(float v); // <--- add this declaration

    // Simple 2D int coord
    struct GridCoord
    {
        int x{ 0 };
        int y{ 0 };

        bool operator==(const GridCoord& rhs) const { return x == rhs.x && y == rhs.y; }
        bool operator!=(const GridCoord& rhs) const { return !(*this == rhs); }
    };

    // Simple 2D float vector (so you don't depend on a math lib)
    struct Vec2
    {
        float x{ 0.0f };
        float y{ 0.0f };
    };

    class Grid
    {
    public:
        // Create a grid with given dimensions and cell size.
        // originWorld = where grid cell (0,0) starts in world space (bottom-left style).
        Grid(int width, int height, float cellSize, Vec2 originWorld = { 0.0f, 0.0f });

        // --- Grid Init Vars --- 
        AEGfxTexture* pTileTex;
        AEGfxVertexList* pTileMesh;


        // -----------------------------
        // GRID SETTINGS
        // -----------------------------
        const int cols = 20;   // how many tiles across
        const int rows = 11;   // how many tiles down
        bool showGrid = true;


        // ---- Basic info ----
        int   GetWidth() const { return m_width; }
        int   GetHeight() const { return m_height; }
        float GetCellSize() const { return m_cellSize; }
        Vec2  GetOriginWorld() const { return m_originWorld; }

        // ---- Coordinate conversions ----
        // Convert world position -> grid coord (floors into cell)
        GridCoord WorldToGrid(Vec2 world) const;

        // Convert grid coord -> world position of the cell center
        Vec2 GridToWorldCenter(GridCoord c) const;

        // Convert grid coord -> world position of the cell bottom-left corner
        Vec2 GridToWorldMin(GridCoord c) const;

        // ---- Bounds ----
        bool InBounds(GridCoord c) const;
        bool InBounds(int x, int y) const;

        // ---- Occupancy ----
        // Place an "object id" on a tile (e.g., tower type or entity id).
        // Returns false if out of bounds OR already occupied.
        bool Place(GridCoord c, int objectId = 1);

        // Remove whatever is on that tile. Returns false if out of bounds OR already empty.
        bool Remove(GridCoord c);

        // Query if tile is occupied (false if out of bounds).
        bool IsOccupied(GridCoord c) const;

        // Get object id at tile; returns 0 if out of bounds or empty.
        int GetObjectId(GridCoord c) const;

        // Clear all tiles
        void Clear();

        void InitScene();

        void Update();

        void Draw();

        void Destroy();

    private:
        int m_width{ 0 };
        int m_height{ 0 };
        float m_cellSize{ 1.0f };
        Vec2 m_originWorld{ 0.0f, 0.0f };

        // 0 = empty, non-zero = occupied (stores object id)
        std::vector<int> m_cells;

    private:
        std::size_t Index(int x, int y) const;
    };
}