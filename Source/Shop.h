#pragma once
#include "AEEngine.h"
#include "Tower.h"
#include <vector>
#include <unordered_map>

namespace TowerHandler {

    struct SpriteUV {
        float u0, v0, u1, v1;
    };

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
        int defIndex = 0;
        bool isEmpty = false;
        bool isLevelTwo = false;
    };

    class Shop {
    public:
        void Init();
        void Update(std::vector<Tower>& activeTowers);
        void Draw();
        void DrawTowerSprites(const std::vector<Tower>& activeTowers) const;
        void RefreshSlots();
        void Exit();
        void AddPoints(int amount) { m_points += amount; }

        bool SpendPoints(int amount) {
            if (m_points < amount) return false;
            m_points -= amount;
            return true;
        }

        bool GetSlotCenter(int slotIdx, float& outX, float& outY) const
        {
            if (slotIdx < 0 || slotIdx >= TOWER_SLOTS) return false;
            outX = slots[slotIdx].x;
            outY = slots[slotIdx].y;
            return true;
        }

        void RestoreSlot(int slotIdx)
        {
            if (slotIdx >= 0 && slotIdx < TOWER_SLOTS)
                slots[slotIdx].isEmpty = false;
        }

        void RefundTower(int slotIdx, int paidCost)
        {
            m_points += paidCost;
            if (slotIdx >= 0 && slotIdx < TOWER_SLOTS && !slots[slotIdx].isLevelTwo)
            {
                if (m_purchaseCount > 0)
                    m_purchaseCount--;
            }
            RestoreSlot(slotIdx);
        }

        int GetTowerSlotCount() const { return TOWER_SLOTS; }
        int GetTowerCost()      const { return TOWER_COST; }

    private:
        int m_points = 100;
        s8  m_uiFont = -1;
        const int TOWER_COST = 25;
        const int REFRESH_COST = 25;
        const int LEVEL2_TOWER_COST = 175;
        int m_purchaseCount = 0;
        int GetCurrentTowerCost() const;

        static const int TOTAL_SLOTS = 7;
        static const int TOWER_SLOTS = 6;

        ShopSlot slots[TOTAL_SLOTS] = {};

        float slotSize = 110.0f;
        float padding = 40.0f;
        float m_uiScale = 1.0f;

        float m_pulseTimer = 0.0f;
        int   m_draggedSlot = -1;

        // Screen shake
        float m_shakeTimer = 0.0f;
        float m_shakeOffsetX = 0.0f;
        float m_shakeOffsetY = 0.0f;
        static constexpr float SHAKE_DURATION = 0.45f;
        static constexpr float SHAKE_MAGNITUDE = 10.0f;
        void TriggerShake() { m_shakeTimer = SHAKE_DURATION; }

        AEGfxVertexList* pCircleMesh = nullptr;
        AEGfxTexture* pSpritesheet = nullptr;
        AEGfxTexture* pSlotTex = nullptr;

        AEGfxTexture* pRefreshSheet = nullptr;
        //static const int REFRESH_SHEET_COLS = 11; 
        //static const int REFRESH_SHEET_ROWS = 11; 
        //static const int REFRESH_ICON_COL = 10; 
        //static const int REFRESH_ICON_ROW = 1;

        static const int SHEET_COLS = 13;
        static const int SHEET_ROWS = 10;

        static const int TOWER_DEF_COUNT = 4;
        static const TowerDef TOWER_DEFS[TOWER_DEF_COUNT];

        std::unordered_map<int, int> m_towerDefIndex;

        void DrawPoints()           const;
        void DrawSlotCosts()        const;
        void DrawLowPointsWarning() const;

        void DrawSpriteAtTex(float cx, float cy, float size,
            int col, int row,
            AEGfxTexture* tex,
            int sheetCols, int sheetRows,
            float r = 1.0f, float g = 1.0f, float b = 1.0f) const;

        SpriteUV GetUVFrom(int col, int row, int sheetCols, int sheetRows) const;
    };

} // namespace TowerHandler