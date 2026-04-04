#include "Shop.h"
#include "Utility.h"
#include <cstdlib>
#include <cstdio>
#include "GameSettings.h"

namespace TowerHandler {

    const TowerDef Shop::TOWER_DEFS[TOWER_DEF_COUNT] = {
        { { 0.1f, 0.9f, 0.2f, 1.0f }, 0, 0, BASIC_TOWER },  // Green  = BASIC
        { { 0.7f, 0.2f, 0.9f, 1.0f }, 2, 0, SNIPER_TOWER }, // Purple = SNIPER
        { { 0.1f, 0.8f, 0.9f, 1.0f }, 4, 0, SLOW_TOWER },   // Cyan   = SLOW
        { { 1.0f, 0.5f, 0.1f, 1.0f }, 6, 0, RAPID_TOWER },  // Orange = RAPID
        { { 1.0f, 0.2f, 0.2f, 1.0f }, 8, 0, BOMB_TOWER },   // Red    = BOMB
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
        SpriteUV uv{};
        uv.u0 = (float)col / (float)sheetCols;
        uv.u1 = (float)(col + 1) / (float)sheetCols;
        uv.v0 = (float)row / (float)sheetRows;
        uv.v1 = (float)(row + 1) / (float)sheetRows;
        return uv;
    }

    // DrawSpriteAtTex – r/g/b allow a colour-multiply tint (default white)
    void Shop::DrawSpriteAtTex(float cx, float cy, float size,
        int col, int row,
        AEGfxTexture* tex,
        int sheetCols, int sheetRows,
        float r, float g, float b) const
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

        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(tex, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(r, g, b, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        AEMtx33 scale, trans, transform;
        AEMtx33Scale(&scale, size, size);
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

        m_uiScale = windowHeight / 900.0f;
        if (m_uiScale > 1.0f) m_uiScale = 1.0f;

        slotSize = 100.0f * m_uiScale;
        padding = 50.0f * m_uiScale; // tighter to fit 7 slots

        float totalWidth = (TOTAL_SLOTS * slotSize) + ((TOTAL_SLOTS - 1) * padding);
        float startX = -(totalWidth / 2.0f) + (slotSize / 2.0f);
        float posY = -(windowHeight / 2.0f) + (slotSize / 2.0f) + 30.0f;

        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            slots[i].x = startX + (i * (slotSize + padding));
            slots[i].y = posY;
            slots[i].size = slotSize;
            slots[i].isRefreshButton = (i == TOWER_SLOTS); // last slot = refresh
        }

        pCircleMesh = BuildCircleMesh(64);
        pSpritesheet = AEGfxTextureLoad("Assets/spritesheet.png");
        pRefreshSheet = AEGfxTextureLoad("Assets/refresh_icon.png");
        pSlotTex = AEGfxTextureLoad("Assets/item_window.png");

        /*
        if (!pSlotTex) PRINT("Shop Init: Failed to load Assets/item_window.png!\n");
        if (!pCircleMesh)  PRINT("Shop Init: Failed to create circle mesh!\n");
        if (!pSpritesheet) PRINT("Shop Init: Failed to load Assets/spritesheet.png!\n");
        if (!pRefreshSheet) PRINT("Refresh Init: Failed to load Assets/refresh_icon.png!\n");
        */

        int fontSize = (int)(24.0f * m_uiScale);
        if (fontSize < 10) fontSize = 10;
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", fontSize);
        m_points = 300;

        RefreshSlots();
        m_sfxRefresh = AEAudioLoadMusic("Assets/PowerUp.wav");
        m_sfxGroup = AEAudioCreateGroup();
    }

    // ----------------------------------------------------------------
    //  RefreshSlots
    //  Fills all 6 tower slots with random tower types, then randomly
    //  display exactly 2 of them at level2.
    // ----------------------------------------------------------------
    void Shop::RefreshSlots()
    {
        for (int i = 0; i < TOWER_SLOTS; ++i)
        {
            slots[i].defIndex = rand() % TOWER_DEF_COUNT;
            slots[i].isEmpty = false;
            slots[i].isLevelTwo = false;
        }

        // Pick 2 distinct NON-BOMB slot indices to be level2
        int idx1 = -1;
        int idx2 = -1;

        do
        {
            idx1 = rand() % TOWER_SLOTS;
        } while (TOWER_DEFS[slots[idx1].defIndex].type == BOMB_TOWER);

        do
        {
            idx2 = rand() % TOWER_SLOTS;
        } while (idx2 == idx1 || TOWER_DEFS[slots[idx2].defIndex].type == BOMB_TOWER);

        slots[idx1].isLevelTwo = true;
        slots[idx2].isLevelTwo = true;

        m_purchaseCount = 0; // reset escalating cost back to original TOWER_COST
    }

    //  GetCurrentTowerCost  (level1 slots only)
    int Shop::GetCurrentTowerCost() const
    {
        int cost = (m_purchaseCount + 1) * TOWER_COST;
        if (cost > 100) cost = 100;
        return cost;
    }

    // Update
    void Shop::Update(std::vector<Tower>& activeTowers)
    {
        float mouseX, mouseY;
        Utility::GetWorldMousePos(mouseX, mouseY);

        float dt = (float)AEFrameRateControllerGetFrameTime();
        m_pulseTimer += dt;

        // Tick screen shake
        if (m_shakeTimer > 0.0f)
        {
            m_shakeTimer -= dt;
            if (m_shakeTimer <= 0.0f)
            {
                m_shakeTimer = 0.0f;
                m_shakeOffsetX = 0.0f;
                m_shakeOffsetY = 0.0f;
            }
            else
            {
                float progress = m_shakeTimer / SHAKE_DURATION; // 1→0 as shake dies
                m_shakeOffsetX = sinf(m_pulseTimer * 40.0f) * SHAKE_MAGNITUDE * progress;
                m_shakeOffsetY = sinf(m_pulseTimer * 40.0f + 1.5708f) * SHAKE_MAGNITUDE * progress * 0.5f;
            }
        }

        //// Debug: add points
        //if (AEInputCheckTriggered(AEVK_M))
        //    m_points += 100;

        // Track which slot (if any) has a tower currently being dragged
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

                if (Utility::IsCircleClicked(slots[slotIdx].x, slots[slotIdx].y,
                    slots[slotIdx].size / 2.0f, mouseX, mouseY))
                {
                    RefundTower(slotIdx, t.purchaseCost);

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

                if (slots[i].isRefreshButton)
                {
                    if (m_points < REFRESH_COST)
                    {
                        // PRINT("Not enough points to refresh! Need %d\n", REFRESH_COST);
                        TriggerShake();
                        return;
                    }
                    m_points -= REFRESH_COST;
                    RefreshSlots();
                    float sfxVol = GameSettings::masterVolume / 100.0f;
                    AEAudioPlay(m_sfxRefresh, m_sfxGroup, sfxVol, 1.0f, 0);
                    return;
                }

                if (slots[i].isEmpty) return;

                const TowerDef& def = TOWER_DEFS[slots[i].defIndex];

                // Determine cost based on tower type + tier
                int cost = 0;

                if (def.type == BOMB_TOWER)
                {
                    cost = BOMB_TOWER_COST;
                }
                else if (slots[i].isLevelTwo)
                {
                    cost = LEVEL2_TOWER_COST;
                }
                else
                {
                    cost = GetCurrentTowerCost();
                }

                if (m_points < cost)
                {
                    // PRINT("Not enough points! Need %d\n", cost);
                    TriggerShake();
                    return;
                }

                m_points -= cost;

                ShopTower tempShop;
                tempShop.ShopTowerInit(mouseX, mouseY, 55.0f, 55.0f, def.type);

                Tower newTower;
                newTower.TowerInit(mouseX, mouseY, 55.0f, 55.0f, tempShop);
                newTower.purchaseCost = cost;
                newTower.color = def.color;
                newTower.isDragging = true;
                newTower.sourceSlotIndex = i;

                if (tempShop.spriteId != 0)
                {
                    Graphics::Destroy(tempShop.spriteId);
                    tempShop.spriteId = 0;
                }

                // Level2 slots produce a tower that is already levelled up
                if (slots[i].isLevelTwo && def.type != BOMB_TOWER)
                    newTower.LevelUp();

                m_towerDefIndex[newTower.details.ID] = slots[i].defIndex;

                activeTowers.push_back(newTower);
                slots[i].isEmpty = true;

                // Only advance the increasing cost counter for level1 purchases
                if (!slots[i].isLevelTwo && def.type != BOMB_TOWER)
                    m_purchaseCount++;

                break;
            }
        }
    }

    //  DrawTowerSprites
    void Shop::DrawTowerSprites(const std::vector<Tower>& activeTowers) const
    {
        (void)activeTowers;
    }

    //  Draw
    void Shop::Draw()
    {
        // Gold tint constants for level2 slots
        const float GOLD_R = 1.0f, GOLD_G = 0.85f, GOLD_B = 0.25f;

        // 1: slot backgrounds – gold tint for level2 slots
        if (pSlotTex)
        {
            for (int i = 0; i < TOTAL_SLOTS; ++i)
            {
                bool gold = !slots[i].isRefreshButton && slots[i].isLevelTwo;
                DrawSpriteAtTex(slots[i].x + m_shakeOffsetX, slots[i].y + m_shakeOffsetY, slots[i].size,
                    0, 0, pSlotTex, 1, 1,
                    gold ? GOLD_R : 1.0f,
                    gold ? GOLD_G : 1.0f,
                    gold ? GOLD_B : 1.0f);
            }
        }

        // 2: tower sprites inside each slot
        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (slots[i].isRefreshButton) continue;
            if (slots[i].isEmpty)         continue;

            const TowerDef& def = TOWER_DEFS[slots[i].defIndex];
            int col = TowerHandler::TOWER_SPRITE_COLS[(int)def.type];
            int row = slots[i].isLevelTwo ? 1 : 0;
            if (def.type == BOMB_TOWER)
                row = 0;

            DrawSpriteAtTex(slots[i].x + m_shakeOffsetX, slots[i].y + m_shakeOffsetY, slots[i].size * 0.55f,
                col, row, pSpritesheet, SHEET_COLS, SHEET_ROWS);
        }

        // 3: refresh icon
        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (!slots[i].isRefreshButton) continue;
            DrawSpriteAtTex(slots[i].x + m_shakeOffsetX, slots[i].y + m_shakeOffsetY, slots[i].size * 0.65f,
                0, 0,                 // always 0,0
                pRefreshSheet,
                1, 1);                // IMPORTANT: full texture
        }

        DrawSlotCosts();
        DrawPoints();
        DrawLowPointsWarning();
    }

    //  DrawSlotCosts
    void Shop::DrawSlotCosts() const
    {
        if (m_uiFont < 0) return;

        const float screenW = (float)AEGfxGetWindowWidth();
        const float screenH = (float)AEGfxGetWindowHeight();

        const float offsetX = slots[0].size * 0.05f;
        const float offsetY = slots[0].size * 0.20f;

        for (int i = 0; i < TOTAL_SLOTS; ++i)
        {
            if (!slots[i].isRefreshButton && slots[i].isEmpty) continue;

            int cost;
            float cr, cg, cb; // label colour
            if (slots[i].isRefreshButton)
            {
                cost = REFRESH_COST;
                cr = 1.0f; cg = 1.0f; cb = 0.0f; // yellow
            }
            else
            {
                const TowerDef& def = TOWER_DEFS[slots[i].defIndex];

                if (def.type == BOMB_TOWER)
                {
                    cost = BOMB_TOWER_COST;
                    cr = 1.0f; cg = 1.0f; cb = 0.0f; // yellow
                }
                else if (slots[i].isLevelTwo)
                {
                    cost = LEVEL2_TOWER_COST;
                    cr = 1.0f; cg = 0.6f; cb = 0.1f; // orange-gold
                }
                else
                {
                    cost = GetCurrentTowerCost();
                    cr = 1.0f; cg = 1.0f; cb = 0.0f; // yellow
                }
            }

            float worldX = slots[i].x + offsetX + m_shakeOffsetX;
            float worldY = slots[i].y + offsetY + m_shakeOffsetY;
            float screenX = worldX + screenW * 0.5f;
            float screenY = screenH * 0.5f - worldY;
            float normX = (screenX / screenW) * 2.0f - 1.0f;
            float normY = 1.0f - (screenY / screenH) * 2.0f;

            char buf[8];
            sprintf_s(buf, "%d", cost);
            AEGfxPrint(m_uiFont, buf, normX, normY, 0.55f, cr, cg, cb, 1.0f);
        }
    }

    //  DrawLowPointsWarning
    //  Shows a flashing "NOT ENOUGH POINTS!" banner at screen centre
    void Shop::DrawLowPointsWarning() const
    {
        if (m_shakeTimer <= 0.0f) return;
        if (m_uiFont < 0)   return;

        // Flash: visible for ~0.6 s out of every 1.0 s cycle
        float cycle = fmodf(m_pulseTimer, 1.0f);
        if (cycle > 0.6f) return;

        // Intensity pulses from 1 → 0.5 across the visible window
        float intensity = 0.5f + 0.5f * (1.0f - cycle / 0.6f);

        const float screenW = (float)AEGfxGetWindowWidth();
        const float screenH = (float)AEGfxGetWindowHeight();

        const char* msg = "NOT ENOUGH POINTS!";
        const float scale = 1.4f;
        const float charW = 22.0f * scale;
        float textPixelWidth = (float)strlen(msg) * charW;
        float px = screenW * 0.5f - textPixelWidth * 0.5f;
        float py = screenH * 0.42f; // slightly above centre

        float normX = (px / screenW) * 2.0f - 1.0f;
        float normY = 1.0f - (py / screenH) * 2.0f;

        // Bright red, fading with pulse
        AEGfxPrint(m_uiFont, msg, normX, normY, scale, intensity, 0.1f, 0.1f, 1.0f);
    }

    //  DrawPoints
    void Shop::DrawPoints() const
    {
        if (m_uiFont < 0) return;
        char buf[32];
        sprintf_s(buf, "POINTS: %d", m_points);

        float screenW = (float)AEGfxGetWindowWidth();
        float screenH = (float)AEGfxGetWindowHeight();

        float px = screenW * 0.78f;
        float py = screenH * 0.04f;
        float normX = (px / screenW) * 2.0f - 1.0f;
        float normY = 1.0f - (py / screenH) * 2.0f;

        AEGfxPrint(m_uiFont, buf, normX, normY, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    }

    //  Exit
    void Shop::Exit()
    {
        if (pCircleMesh) { AEGfxMeshFree(pCircleMesh);        pCircleMesh = nullptr; }
        if (pSlotTex) { AEGfxTextureUnload(pSlotTex);       pSlotTex = nullptr; }
        if (pSpritesheet) { AEGfxTextureUnload(pSpritesheet);   pSpritesheet = nullptr; }
        if (pRefreshSheet) { AEGfxTextureUnload(pRefreshSheet);  pRefreshSheet = nullptr; }
        if (m_uiFont >= 0) { AEGfxDestroyFont(m_uiFont);         m_uiFont = -1; }
        m_towerDefIndex.clear();
    }

} // namespace TowerHandler