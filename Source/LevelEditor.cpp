#include "LevelEditor.h"
#include "AEInput.h"
#include <cstdio>   // for sprintf_s
#include <cctype>   // for std::tolower

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

    // NOTE: Hardcoded for now
    const int texW = 80;
    const int texH = 160;

    m_tilesetCols = texW / tilePx;
    m_tilesetRows = texH / tilePx;

    // Valid tile IDs: 1..(cols*rows). 0 = empty
    m_maxTileId = m_tilesetCols * m_tilesetRows;

    m_tileMeshes.assign((size_t)m_maxTileId + 1, nullptr);

    // Default file name shown in editor
    saveFileName = "level_01";
    m_isTypingFileName = false;
    m_fileNameBeforeTyping = saveFileName;
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
    if (v > 7) v = 0;
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

void LevelEditor::HandleFileNameTyping()
{
    // A-Z -> a-z
    for (int key = AEVK_A; key <= AEVK_Z; ++key)
    {
        if (AEInputCheckTriggered(key))
        {
            char ch = (char)('a' + (key - AEVK_A));
            saveFileName.push_back(ch);
        }
    }

    // 0-9
    for (int key = AEVK_0; key <= AEVK_9; ++key)
    {
        if (AEInputCheckTriggered(key))
        {
            char ch = (char)('0' + (key - AEVK_0));
            saveFileName.push_back(ch);
        }
    }

    // Numpad 0-9
    for (int key = AEVK_NUMPAD0; key <= AEVK_NUMPAD9; ++key)
    {
        if (AEInputCheckTriggered(key))
        {
            char ch = (char)('0' + (key - AEVK_NUMPAD0));
            saveFileName.push_back(ch);
        }
    }

    // underscore using minus key
    if (AEInputCheckTriggered(AEVK_MINUS))
        saveFileName.push_back('_');

    // hyphen using slash key alternative is ugly; easiest is use period for dot if wanted
    // if you prefer actual '-' instead of '_', replace above with '-'

    // Optional dot
    if (AEInputCheckTriggered(AEVK_PERIOD))
        saveFileName.push_back('.');

    // Backspace
    if (AEInputCheckTriggered(AEVK_BACK) && !saveFileName.empty())
        saveFileName.pop_back();

    // Enter = confirm
    if (AEInputCheckTriggered(AEVK_RETURN))
    {
        if (saveFileName.empty())
            saveFileName = "untitled";

        m_isTypingFileName = false;
    }

    // Escape = cancel and restore old name
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        saveFileName = m_fileNameBeforeTyping;
        m_isTypingFileName = false;
    }
}

void LevelEditor::Update(float /*dt*/)
{
    // Enter filename typing mode
    if (AEInputCheckTriggered(AEVK_F5))
    {
        m_isTypingFileName = true;
        m_fileNameBeforeTyping = saveFileName;
    }

    // If typing file name, ignore normal editor hotkeys
    if (m_isTypingFileName)
    {
        HandleFileNameTyping();
        return;
    }

    // Paint
    if (AEInputCheckCurr(AEVK_LBUTTON)) PaintAtMouse(false);
    if (AEInputCheckCurr(AEVK_RBUTTON)) PaintAtMouse(true);

    // Layer switch
    if (AEInputCheckTriggered(AEVK_F1)) m_layer = ActiveLayer::MapLayer;
    if (AEInputCheckTriggered(AEVK_F2)) m_layer = ActiveLayer::RegionLayer;

    // Save / Load using current typed file name
    if (AEInputCheckTriggered(AEVK_F3))
    {
        std::string fullName = saveFileName;
        if (fullName.empty())
            fullName = "untitled";

        fullName += ".txt";
        m_level.Save("../../Assets/Levels/" + fullName);
    }

    if (AEInputCheckTriggered(AEVK_F4))
    {
        std::string fullName = saveFileName;
        if (fullName.empty())
            fullName = "untitled";

        fullName += ".txt";
        m_level.Load(fullName);
    }

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
}

void LevelEditor::DrawMapOverlay() const
{
    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            int id = m_level.map[m_level.Idx(x, y)];
            if (id == 0) continue;

            m_grid->DrawTileTinted({ x, y }, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void LevelEditor::DrawRegionOverlay() const
{
    for (int y = 0; y < m_level.height; ++y)
    {
        for (int x = 0; x < m_level.width; ++x)
        {
            RegionFlag f = (RegionFlag)m_level.region[m_level.Idx(x, y)];
            if (f == RegionFlag::NONE) continue;

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
        DrawMapTiles(1.0f);
    }
    else
    {
        DrawMapTiles(0.25f);
        DrawRegionNumbers();
    }

    // Hover highlight
    int mx = 0, my = 0;
    AEInputGetCursorPosition(&mx, &my);
    GridSystem::GridCoord c;
    if (m_grid->ScreenToGrid(mx, my, c))
        m_grid->DrawTileTinted(c, 1, 1, 1, 0.75f);

    DrawUI();
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

    const float sel = 1.0f;
    const float dim = 0.35f;
    const float info = 0.65f;
    const float typing = 1.0f;

    const float l1 = mapSelected ? sel : dim;
    const float l2 = mapSelected ? dim : sel;

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

    // Layers
    PrintLine("(F1)  Layer 1 : Map", l1);
    PrintLine("(F2)  Layer 2 : Region", l2);

    // Current tile brush
    if (m_layer == ActiveLayer::MapLayer)
    {
        char buf[64];
        if (m_currentTileId == 0)
            sprintf_s(buf, "Brush (Tile): EMPTY");
        else
            sprintf_s(buf, "Brush (Tile): %d", m_currentTileId);

        PrintLine(buf, info);
    }

    // File name display
    {
        char fileBuf[256];
        if (m_isTypingFileName)
            sprintf_s(fileBuf, "File Name: %s_", saveFileName.c_str());
        else
            sprintf_s(fileBuf, "File Name: %s", saveFileName.c_str());

        PrintLine(fileBuf, typing);
    }

    // Controls
    PrintLine("(Q/E) Cycle brush", info);
    PrintLine("(LMB) Paint", info);
    PrintLine("(RMB) Erase", info);
    PrintLine("(F3)  Save current file", info);
    PrintLine("(F4)  Load current file", info);
    PrintLine("(F5)  Edit file name", info);

    if (m_isTypingFileName)
    {
        PrintLine("Typing Mode:", typing);
        PrintLine("  A-Z / 0-9 to type", typing);
        PrintLine("  Backspace = delete", typing);
        PrintLine("  Enter = confirm", typing);
        PrintLine("  Esc = cancel", typing);
    }

    py += 8.0f;
    PrintLine("Region Numbers:", info);
    PrintLine("  0 = NONE", info);
    PrintLine("  1 = BUILDABLE", info);
    PrintLine("  2 = ENEMYSPAWN", info);
    PrintLine("  3 = ENEMYGOAL", info);
    PrintLine("  4 = ENEMYPATH_UP", info);
    PrintLine("  5 = ENEMYPATH_DOWN", info);
    PrintLine("  6 = ENEMYPATH_LEFT", info);
    PrintLine("  7 = ENEMYPATH_RIGHT", info);
}