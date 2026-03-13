#include "Shop.h"
#include "Utility.h"
#include <cstdlib>
#include <cstdio>

namespace TowerHandler {

    const TowerDef Shop::TOWER_DEFS[TOWER_DEF_COUNT] = {
        { { 0.1f, 0.9f, 0.2f, 1.0f }, 0, 0, BASIC_TOWER },  // Green  = BASIC
        { { 0.7f, 0.2f, 0.9f, 1.0f }, 2, 0, SNIPER_TOWER }, // Purple = SNIPER
        { { 0.1f, 0.8f, 0.9f, 1.0f }, 4, 0, SLOW_TOWER },   // Cyan   = SLOW
        { { 1.0f, 0.5f, 0.1f, 1.0f }, 6, 0, RAPID_TOWER },  // Orange = RAPID
    };

    static AEGfxVertexList* BuildCircleMesh(int segments)
    {
        AEGfxMeshStart();
        const float step = 2.0f * 3.14159f / (float)segments;
        for (int i = 0; i < segments; ++i)
        {
            float a0 = step * (float)i;
            float a1 = step * (float)(i + 1);
            float x0 = cosf(a0), y0 = sinf(a0);
            float x1 = cosf(a1), y1 = sinf(a1);
            unsigned int col = 0xFFFFFFFF;
            AEGfxTriAdd(0.0f, 0.0f, col, 0.5f, 0.5f,
                x0, y0, col, 0.5f + 0.5f * x0, 0.5f - 0.5f * y0,
                x1, y1, col, 0.5f + 0.5f * x1, 0.5f - 0.5f * y1);
        }
        return AEGfxMeshEnd();
    }

    SpriteUV Shop::GetUVFrom(int col, int row, int sheetCols, int sheetRows) const
    {
        SpriteUV uv;
        uv.u0 = (float)col / (float)sheetCols;
        uv.u1 = (float)(col + 1) / (float)sheetCols;
        uv.v0 = (float)row / (float)sheetRows;
        uv.v1 = (float)(row + 1) / (float)sheetRows;
        return uv;
    }

    // DrawSpriteAtTex  (generic caller supplies texture + sheet dimensions)
    void Shop::DrawSpriteAtTex(float cx, float cy, float size,
        int col, int row,
        AEGfxTexture* tex,
        int sheetCols, int sheetRows) const
    {
        if (!tex) return;

        SpriteUV uv = GetUVFrom(col, row, sheetCols, sheetRows);

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, uv.u0, uv.v1,
            0.5f, -0.5f, 0xFFFFFFFF, uv.u1, uv.v1,
            0.5f, 0.5f, 0xFFFFFFFF, uv.u1, uv.v0);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, uv.u0, uv.v1,
            0.5f, 0.5f, 0xFFFFFFFF, uv.u1, uv.v0,
            -0.5f, 0.5f, 0xFFFFFFFF, uv.u0, uv.v0);
        AEGfxVertexList* mesh = AEGfxMeshEnd();
        if (!mesh) return;

        float spriteSize = size * 0.80f;

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(tex, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        AEMtx33 scale, trans, transform;
        AEMtx33Scale(&scale, spriteSize, spriteSize);
        AEMtx33Trans(&trans, cx, cy);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(mesh);
    }

    // Init
    void Shop::Init()
    {
        float windowHeight = (float)AEGfxGetWindowHeight();

        float totalWidth = (TOTAL_SLOTS * slotSize) + ((TOTAL_SLOTS - 1) * padding);
        float startX = -(totalWidth / 2.0f) + (slotSize / 2.0f);
        float posY = -(windowHeight / 2.0f) + (slotSize / 2.0f) + 30.0f;

        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            slots[i].x = startX + (i * (slotSize + padding));
            slots[i].y = posY;
            slots[i].size = slotSize;
            slots[i].isRefreshButton = (i == TOWER_SLOTS);
        }

        pCircleMesh = BuildCircleMesh(64);
        pSpritesheet = AEGfxTextureLoad("Assets/spritesheet.png");
        pRefreshSheet = AEGfxTextureLoad("Assets/refresh_overlay.png");

        if (!pCircleMesh)  PRINT("Shop Init: Failed to create circle mesh!\n");
        if (!pSpritesheet) PRINT("Shop Init: Failed to load Assets/spritesheet.png!\n");
        if (!pRefreshSheet) PRINT("Refresh Init: Failed to load Assets/refresh_overlay.png!\n");

        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
        m_points = 1000;

        RefreshSlots();
    }

    // RefreshSlots
    void Shop::RefreshSlots()
    {
        int idx[TOWER_DEF_COUNT];
        for (int i = 0; i < TOWER_DEF_COUNT; ++i) idx[i] = i;
        for (int i = TOWER_DEF_COUNT - 1; i > 0; --i)
        {
            int j = rand() % (i + 1);
            int tmp = idx[i]; idx[i] = idx[j]; idx[j] = tmp;
        }

        for (int i = 0; i < TOWER_SLOTS; ++i) {
            slots[i].defIndex = idx[i];
            slots[i].isEmpty = false;
        }
    }

    // Update
    void Shop::Update(std::vector<Tower>& activeTowers)
    {
        float mouseX, mouseY;
        Utility::GetWorldMousePos(mouseX, mouseY);

        // Advance pulse timer
        m_pulseTimer += (float)AEFrameRateControllerGetFrameTime();

        // Find which slot (if any) has its tower currently being dragged
        m_draggedSlot = -1;
        for (const TowerHandler::Tower& t : activeTowers)
        {
            if (t.isDragging && t.sourceSlotIndex >= 0 && t.sourceSlotIndex < TOWER_SLOTS)
            {
                m_draggedSlot = t.sourceSlotIndex;
                break;
            }
        }

        if (!AEInputCheckCurr(AEVK_LBUTTON))
        {
            for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
            {
                TowerHandler::Tower& t = activeTowers[(size_t)i];
                if (!t.isDragging) continue;

                int slotIdx = t.sourceSlotIndex;
                if (slotIdx < 0 || slotIdx >= TOWER_SLOTS) continue;

                // Check if mouse is over the source slot
                if (Utility::IsCircleClicked(slots[slotIdx].x, slots[slotIdx].y,
                    slots[slotIdx].size / 2.0f, mouseX, mouseY))
                {
                    // Refund points and restore slot
                    m_points += TOWER_COST;
                    slots[slotIdx].isEmpty = false;

                    // Remove the tower's Graphics sprite
                    if (t.spriteId != 0)
                        Graphics::Destroy(t.spriteId);
                    t.Destroy();

                    activeTowers.erase(activeTowers.begin() + i);
                }
            }
        }

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            for (int i = 0; i < TOTAL_SLOTS; ++i)
            {
                if (!Utility::IsCircleClicked(slots[i].x, slots[i].y,
                    slots[i].size / 2.0f, mouseX, mouseY))
                    continue;

                if (slots[i].isRefreshButton) { 
                    if (m_points < REFRESH_COST) {
                        PRINT("Not enough points to refresh! Need %d\n", REFRESH_COST);
                        return;
                    }
                    m_points -= REFRESH_COST;
                    RefreshSlots(); 
                    return; 
                }

                if (slots[i].isEmpty) {
                    return;
                }

                if (m_points < TOWER_COST)
                {
                    PRINT("Not enough points! Need %d\n", TOWER_COST);
                    return;
                }

                m_points -= TOWER_COST;

                const TowerDef& def = TOWER_DEFS[slots[i].defIndex];

                // Build a ShopTower so TowerInit gets the correct type
                ShopTower tempShop;
                tempShop.ShopTowerInit(mouseX, mouseY, 55.0f, 55.0f, def.type);

                Tower newTower;
                newTower.TowerInit(mouseX, mouseY, 55.0f, 55.0f, tempShop);
                // Destroy the temporary sprite
                if (tempShop.spriteId != 0)
                {
                    Graphics::Destroy(tempShop.spriteId);
                    tempShop.spriteId = 0;
                }

                newTower.details.spriteCol = def.spriteCol;
                newTower.details.spriteBaseRow = def.spriteRow;

                // Override color to match the shop slot (TowerInit sets generic type color)
                newTower.color = def.color;

                // Override the Graphics sprite UV to use the TowerDef's exact spriteCol/spriteRow
                // TowerInit defaults to col=(int)towerType 
                UVRect uv = GetSpriteUV(def.spriteCol, def.spriteRow, 13, 10);
                Graphics::SetUV(newTower.spriteId, uv.u0, uv.v0, uv.u1, uv.v1);

                newTower.isDragging = true;
                newTower.sourceSlotIndex = i;

                // Map tower ID -> def index so DrawTowerSprites can find the right sprite
                m_towerDefIndex[newTower.details.ID] = slots[i].defIndex;

                activeTowers.push_back(newTower);
                slots[i].isEmpty = true;
                break;
            }
        }
    }

    // DrawTowerSprites - call AFTER all towers are drawn each frame
    void Shop::DrawTowerSprites(const std::vector<Tower>& activeTowers) const
    {
        // Tower sprites are now handled via Graphics::RenderAll() in the scene's Draw().
        // Each tower's spriteId was assigned the correct UV from the TowerDef on spawn,
        // so no separate draw pass is needed here.
        (void)activeTowers;
    }

    // Draw - shop UI only
    void Shop::Draw()
    {
        if (!pCircleMesh) return;

        // 1: colored circles
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            // Pulse the slot when its tower is being dragged out
            float drawSize = slots[i].size;
            if (i == m_draggedSlot)
                drawSize *= 1.0f + 0.12f * sinf(m_pulseTimer * 6.0f);

            AEMtx33 scale, trans, transform;
            AEMtx33Scale(&scale, slots[i].size, slots[i].size);
            AEMtx33Trans(&trans, slots[i].x, slots[i].y);
            AEMtx33Concat(&transform, &trans, &scale);
            AEGfxSetTransform(transform.m);

            if (slots[i].isRefreshButton)
                AEGfxSetColorToMultiply(0.4f, 0.4f, 0.4f, 1.0f);
            else if (slots[i].isEmpty)
                AEGfxSetColorToMultiply(0.15f, 0.15f, 0.15f, 0.15f);
            else
            {
                const Color& c = TOWER_DEFS[slots[i].defIndex].color;
                AEGfxSetColorToMultiply(c.r, c.g, c.b, 1.0f);
            }

            AEGfxMeshDraw(pCircleMesh, AE_GFX_MDM_TRIANGLES);
        }

        // 2: sprites on top of tower slots
        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (slots[i].isRefreshButton) continue;
            if (slots[i].isEmpty) continue;
            const TowerDef& def = TOWER_DEFS[slots[i].defIndex];
            DrawSpriteAtTex(slots[i].x, slots[i].y, slots[i].size,
                def.spriteCol, def.spriteRow, pSpritesheet, SHEET_COLS, SHEET_ROWS);
        }

        // 3: refresh icon on top of the refresh button
        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (!slots[i].isRefreshButton) continue;
            DrawSpriteAtTex(slots[i].x, slots[i].y, slots[i].size * 1.5f, // enlarge sprite size
                REFRESH_ICON_COL, REFRESH_ICON_ROW,
                pRefreshSheet,
                REFRESH_SHEET_COLS, REFRESH_SHEET_ROWS);
        }

        DrawPoints();
    }

    // DrawPoints
    void Shop::DrawPoints() const
    {
        if (m_uiFont < 0) return;
        char buf[32];
        sprintf_s(buf, "POINTS: %d", m_points);
        AEGfxPrint(m_uiFont, buf, 0.6f, 0.9f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    }

    // Exit
    void Shop::Exit()
    {
        if (pCircleMesh) { AEGfxMeshFree(pCircleMesh);       pCircleMesh = nullptr; }
        if (pSpritesheet) { AEGfxTextureUnload(pSpritesheet);  pSpritesheet = nullptr; }
        if (pRefreshSheet) { AEGfxTextureUnload(pRefreshSheet);  pRefreshSheet = nullptr; }
        if (m_uiFont >= 0) { AEGfxDestroyFont(m_uiFont);        m_uiFont = -1; }
        m_towerDefIndex.clear();
    }

} // namespace TowerHandler