#include "LevelEditor.h"
#include "AEInput.h"
#include <cstdio> // for sprintf_s

// Helper Functions
static float ScreenToNormX(float px)
{
    return (px / AEGfxGetWindowWidth()) * 2.0f - 1.0f;
}

static float ScreenToNormY(float py)
{
    return 1.0f - (py / AEGfxGetWindowHeight()) * 2.0f;
}

static int ClampI(int v, int lo, int hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

void LevelEditor::Init(int w, int h)
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 16);
    m_level.Resize(w, h);

    m_grid = new GridSystem::Grid(w, h, 1.0f);
    m_grid->Init();

    m_tilesetTex = AEGfxTextureLoad("Assets/tilesheet.png");
    if (!m_tilesetTex)
        PRINT("FAILED to load tileset: Assets/tilesheet.png\n");

    const int tilePx = 16;

    // NOTE: use whatever your AE engine provides for texture dimensions.
    // Common names are AEGfxTextureGetWidth/Height, but adjust if yours differ.
    const int texW = 80;
    const int texH = 160;

    m_tilesetCols = texW / tilePx;
    m_tilesetRows = texH / tilePx;

    // Valid tile IDs: 1..(cols*rows). 0 = empty
    m_maxTileId = m_tilesetCols * m_tilesetRows;

    m_tileMeshes.assign((size_t)m_maxTileId + 1, nullptr);
}

void LevelEditor::Shutdown()
{
    if (m_grid)
    {
        m_grid->Destroy();
        delete m_grid;
        m_grid = nullptr;
    }

    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    // free cached tile meshes
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

void LevelEditor::CycleMapTile(int dir)
{
    const int count = m_maxTileId + 1; // includes 0

    m_currentTileId = (m_currentTileId + dir) % count;

    // C++ modulo can go negative, fix it
    if (m_currentTileId < 0)
        m_currentTileId += count;
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

AEGfxVertexList* LevelEditor::GetTileMesh(int tileId)
{
    if (tileId <= 0 || tileId > m_maxTileId) return nullptr;
    if ((int)m_tileMeshes.size() <= tileId) return nullptr;

    if (m_tileMeshes[tileId]) return m_tileMeshes[tileId];

    // tileId=1 is first tile (0,0)
    int index = tileId - 1;
    int tx = index % m_tilesetCols;
    int ty = index / m_tilesetCols;   // ty=0 is FIRST row you sample

    // Flip so row 0 == TOP row of the texture
    ty = (m_tilesetRows - 1) - ty;

    if (ty < 0 || ty >= m_tilesetRows) return nullptr;

    float u0 = (float)tx / (float)m_tilesetCols;
    float u1 = (float)(tx + 1) / (float)m_tilesetCols;

    float v0 = (float)ty / (float)m_tilesetRows;
    float v1 = (float)(ty + 1) / (float)m_tilesetRows;

    // If your tiles appear vertically flipped, swap v0 and v1 below.
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

void LevelEditor::Update(float /*dt*/)
{
    // Paint
    if (AEInputCheckCurr(AEVK_LBUTTON)) PaintAtMouse(false);
    if (AEInputCheckCurr(AEVK_RBUTTON)) PaintAtMouse(true);

    // Layer switch
    if (AEInputCheckTriggered(AEVK_F1)) m_layer = ActiveLayer::MapLayer;
    if (AEInputCheckTriggered(AEVK_F2)) m_layer = ActiveLayer::RegionLayer;
    
    // Save / Load
    if (AEInputCheckTriggered(AEVK_F3)) m_level.Save(m_path);
    if (AEInputCheckTriggered(AEVK_F4)) m_level.Load(m_path);

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

    Draw();
    DrawUI();
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

void LevelEditor::DrawRegionNumbers() const
{
    if (m_uiFont < 0) return;
    if (!m_grid) return;

    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();

    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            RegionFlag f = (RegionFlag)m_level.region[m_level.Idx(x, y)];
            if (f == RegionFlag::NONE) continue;

            float wx, wy, tileSize;
            m_grid->GetCellWorldCenter({ x, y }, wx, wy, tileSize);

            // world -> screen pixels (top-left origin)
            float px = wx + screenW * 0.5f;
            float py = screenH * 0.5f - wy;

            // slight nudge so it looks centered
            px -= 6.0f;
            py -= 10.0f;

            float tx = ScreenToNormX(px);
            float ty = ScreenToNormY(py);

            char buf[8];
            sprintf_s(buf, "%d", (int)f);

            AEGfxPrint(m_uiFont, buf, tx, ty, 1.0f, 1, 1, 1, 1);
        }
    }
}

void LevelEditor::DrawMapTiles(float alphaMul) const
{
    if (!m_tilesetTex || !m_grid) return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_tilesetTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0, 0, 0, 0);
    AEGfxSetColorToMultiply(1, 1, 1, 1);

    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            int id = m_level.map[m_level.Idx(x, y)];
            if (id == 0) continue;

            // Build/reuse the UV mesh for this tile id
            AEGfxVertexList* mesh = const_cast<LevelEditor*>(this)->GetTileMesh(id);
            if (!mesh) continue;

            float wx, wy, tileSize;
            m_grid->GetCellWorldCenter({ x, y }, wx, wy, tileSize);

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

void LevelEditor::Draw()
{
    if (!m_grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    // Draw base grid
    m_grid->Draw();

    if (m_layer == ActiveLayer::MapLayer)
    {
        // Layer 1: draw tiles normally
        DrawMapTiles(1.0f);
    }
    else
    {
        // Layer 2: dim tiles, then show region numbers
        DrawMapTiles(0.25f);
        DrawRegionNumbers();
    }

    // Hover highlight (still ok)
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);
    GridSystem::GridCoord c;
    if (m_grid->ScreenToGrid(mx, my, c))
        m_grid->DrawTileTinted(c, 1, 1, 1, 0.75f);
}

void LevelEditor::DrawUI() const
{
    const float pad = 25.0f;

    // Anchor in SCREEN PIXELS (top-left origin)
    const float px = pad;
    float py = pad;

    // Vertical spacing between lines (pixels)
    const float lineGapPx = 22.0f;

    // Selected vs unselected colors
    const bool mapSelected = (m_layer == ActiveLayer::MapLayer);

    const float sel = 1.0f;     // bright
    const float dim = 0.35f;    // darker
    const float info = 0.65f;   // normal info text

    const float l1 = mapSelected ? sel : dim; // Layer 1 bright if MapLayer
    const float l2 = mapSelected ? dim : sel; // Layer 2 bright if RegionLayer

    auto PrintLine = [&](const char* text, float shade)
        {
            float tx = ScreenToNormX(px);
            float ty = ScreenToNormY(py);

            AEGfxPrint(
                m_uiFont,
                text,
                tx, ty,
                1.0f,
                shade, shade, shade, 1.0f
            );

            py += lineGapPx;
        };

    // --- Layers (highlight which one is active) ---
    PrintLine("(F1)  Layer 1 : Map", l1);
    PrintLine("(F2)  Layer 2 : Region", l2);

    // --- CURRENT TILE BRUSH (NEW) ---
    if (m_layer == ActiveLayer::MapLayer)
    {
        char buf[64];
        if (m_currentTileId == 0)
            sprintf_s(buf, "Brush (Tile): EMPTY");
        else
            sprintf_s(buf, "Brush (Tile): %d", m_currentTileId);

        PrintLine(buf, info);
    }

    // --- Controls legend ---
    PrintLine("(Q/E) Cycle brush", info);
    PrintLine("(LMB) Paint", info);
    PrintLine("(RMB) Erase", info);
    PrintLine("(F3)  Save", info);
    PrintLine("(F4)  Load", info);
    py += 8.0f;
    PrintLine("Region Numbers:", info);
    PrintLine("  0 = NONE", info);
    PrintLine("  1 = BUILDABLE", info);
    PrintLine("  2 = ENEMYPATH", info);
}

