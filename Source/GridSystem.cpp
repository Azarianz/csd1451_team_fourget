#include "GridSystem.h"
#include <cmath>   // std::floor

namespace GridSystem {
    float RoundToPixel(float v)
    {
        return floorf(v + 0.5f);
    }

    GridSystem::GridSystem(int width, int height, float cellSize, Vec2 originWorld)
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

    std::size_t GridSystem::Index(int x, int y) const
    {
        return static_cast<std::size_t>(y) * static_cast<std::size_t>(m_width)
            + static_cast<std::size_t>(x);
    }

    bool GridSystem::InBounds(int x, int y) const
    {
        return (x >= 0 && x < m_width && y >= 0 && y < m_height);
    }

    bool GridSystem::InBounds(GridCoord c) const
    {
        return InBounds(c.x, c.y);
    }

    GridCoord GridSystem::WorldToGrid(Vec2 world) const
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

    Vec2 GridSystem::GridToWorldMin(GridCoord c) const
    {
        // bottom-left corner of tile
        Vec2 w;
        w.x = m_originWorld.x + (static_cast<float>(c.x) * m_cellSize);
        w.y = m_originWorld.y + (static_cast<float>(c.y) * m_cellSize);
        return w;
    }

    Vec2 GridSystem::GridToWorldCenter(GridCoord c) const
    {
        Vec2 min = GridToWorldMin(c);
        Vec2 center;
        center.x = min.x + m_cellSize * 0.5f;
        center.y = min.y + m_cellSize * 0.5f;
        return center;
    }

    bool GridSystem::Place(GridCoord c, int objectId)
    {
        if (!InBounds(c)) return false;
        if (objectId == 0) objectId = 1; // keep 0 as "empty"

        const std::size_t idx = Index(c.x, c.y);
        if (m_cells[idx] != 0) return false; // already occupied

        m_cells[idx] = objectId;
        return true;
    }

    bool GridSystem::Remove(GridCoord c)
    {
        if (!InBounds(c)) return false;

        const std::size_t idx = Index(c.x, c.y);
        if (m_cells[idx] == 0) return false; // already empty

        m_cells[idx] = 0;
        return true;
    }

    bool GridSystem::IsOccupied(GridCoord c) const
    {
        if (!InBounds(c)) return false;
        return m_cells[Index(c.x, c.y)] != 0;
    }

    int GridSystem::GetObjectId(GridCoord c) const
    {
        if (!InBounds(c)) return 0;
        return m_cells[Index(c.x, c.y)];
    }

    void GridSystem::Clear()
    {
        for (int& v : m_cells)
            v = 0;
    }
}