#pragma once
#include "AEEngine.h"
#include "Tower.h"
#include <vector>
#include <unordered_map>

namespace TowerHandler {

    struct SpriteUV {
        float u0, v0, u1, v1;
    };

    // ----------------------------------------------------------------
    // TowerDef — binds a color, sprite, and tower type together.
    // Edit this table to control what each "kind" of tower looks like.
    // spriteCol/spriteRow are 0-based indices into the spritesheet
    // (208x160 px, 16px tiles => 13 cols x 10 rows).
    // ----------------------------------------------------------------
    struct TowerDef {
        Color     color;
        int       spriteCol;
        int       spriteRow;
        TowerType type;
    };

    struct ShopSlot {
        float x, y;
        float size;
        bool isRefreshButton;

        // Index into the shared TowerDef table — drives color AND sprite
        int defIndex = 0;
    };

    class Shop {
    public:
        void Init();
        void Update(std::vector<Tower>& activeTowers);

        // Draw shop UI (slots + points)
        void Draw();

        // Draw sprites locked to all active towers — call AFTER towers are drawn
        void DrawTowerSprites(const std::vector<Tower>& activeTowers) const;

        void RefreshSlots();
        void Exit();

        void AddPoints(int amount) { m_points += amount; }

    private:
        int m_points = 100;
        s8  m_uiFont = -1;
        const int TOWER_COST = 25;

        static const int TOTAL_SLOTS = 6;   // 5 tower slots + 1 refresh button
        static const int TOWER_SLOTS = 5;

        ShopSlot slots[TOTAL_SLOTS];

        float slotSize = 60.0f;
        float padding = 60.0f;

        AEGfxVertexList* pCircleMesh = nullptr;
        AEGfxTexture* pSpritesheet = nullptr;

        // Sheet layout — spritesheet.png is 208x160, 16px tiles = 13 cols x 10 rows
        static const int SHEET_COLS = 13;
        static const int SHEET_ROWS = 10;

        // ----------------------------------------------------------------
        // Tower definition table — add / edit entries here freely.
        // Each entry is one distinct tower "kind".
        // ----------------------------------------------------------------
        static const int TOWER_DEF_COUNT = 8;
        static const TowerDef TOWER_DEFS[TOWER_DEF_COUNT];

        // Maps tower ID -> def index so the sprite always matches the color
        std::unordered_map<int, int> m_towerDefIndex;

        void DrawPoints() const;
        void DrawSpriteAt(float cx, float cy, float size, int col, int row) const;
        SpriteUV GetUV(int col, int row) const;
    };
}