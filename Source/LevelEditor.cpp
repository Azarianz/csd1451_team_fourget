#include "LevelEditor.h"
#include "AEInput.h"

static int ClampI(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void LevelEditor::Init(int w, int h)
{
    m_level.Resize(w, h);

    m_grid = new GridSystem::Grid(w, h, 1.0f);
    m_grid->Init();
}

void LevelEditor::Shutdown()
{
    if (m_grid)
    {
        m_grid->Destroy();
        delete m_grid;
        m_grid = nullptr;
    }
}

void LevelEditor::CycleMapTile(int dir)
{
    // keep it simple: 0..9 (0 is "empty")
    m_currentTileId = ClampI(m_currentTileId + dir, 0, 9);
}

void LevelEditor::CycleRegion(int dir)
{
    int v = (int)m_currentRegion;
    v += dir;
    if (v < 0) v = 2;
    if (v > 2) v = 0;
    m_currentRegion = (RegionFlag)v;
}

void LevelEditor::PaintAtMouse(bool erase)
{
    if (!m_grid) return;

    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);

    GridSystem::GridCoord c;
    if (!m_grid->ScreenToGrid(mx, my, c)) return;

    if (m_layer == ActiveLayer::MapLayer)
    {
        int value = erase ? 0 : m_currentTileId;
        m_level.map[m_level.Idx(c.x, c.y)] = value;
    }
    else
    {
        uint8_t value = erase ? (uint8_t)RegionFlag::NONE : (uint8_t)m_currentRegion;
        m_level.region[m_level.Idx(c.x, c.y)] = value;
    }
}

void LevelEditor::Update(float /*dt*/)
{
    // Layer switch
    if (AEInputCheckTriggered(AEVK_1)) m_layer = ActiveLayer::MapLayer;
    if (AEInputCheckTriggered(AEVK_2)) m_layer = ActiveLayer::RegionLayer;

    // Cycle brush
    if (AEInputCheckTriggered(AEVK_Q))
    {
        if (m_layer == ActiveLayer::MapLayer) CycleMapTile(-1);
        else CycleRegion(-1);
    }
    if (AEInputCheckTriggered(AEVK_E))
    {
        if (m_layer == ActiveLayer::MapLayer) CycleMapTile(+1);
        else CycleRegion(+1);
    }

    // Paint
    if (AEInputCheckCurr(AEVK_LBUTTON)) PaintAtMouse(false);
    if (AEInputCheckCurr(AEVK_RBUTTON)) PaintAtMouse(true);

    // Save / Load
    if (AEInputCheckTriggered(AEVK_F5))
        m_level.Save(m_path);

    if (AEInputCheckTriggered(AEVK_F9))
        m_level.Load(m_path);

    Draw();
}

void LevelEditor::DrawMapOverlay() const
{
    // MapLayer: any non-zero tile is "painted" -> highlight it
    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            int id = m_level.map[m_level.Idx(x, y)];
            if (id == 0) continue;

            // Opacity-only highlight
            m_grid->DrawTileTinted({ x, y }, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void LevelEditor::DrawRegionOverlay() const
{
    // NONE = nothing
    // BUILDABLE = green tint
    // ENEMYPATH = red tint
    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            RegionFlag f = (RegionFlag)m_level.region[m_level.Idx(x, y)];
            if (f == RegionFlag::NONE) continue;

            // Opacity-only highlight
            m_grid->DrawTileTinted({ x, y }, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void LevelEditor::Draw()
{
    if (!m_grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f); // dark bluish-grey

    // Base grid
    m_grid->Draw();

    // Overlays
    DrawMapOverlay();
    DrawRegionOverlay();

    // (Optional) highlight hovered cell stronger
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);
    GridSystem::GridCoord c;
    if (m_grid->ScreenToGrid(mx, my, c))
        m_grid->DrawTileTinted(c, 1, 1, 1, 0.75f);
}