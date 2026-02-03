#pragma once
#include "AEEngine.h"
#include "Tower.h"
#include <vector>

namespace TowerHandler {
    struct ShopSlot {
        float x, y;
        float size;
        TowerType typeContained;
        bool isRefreshButton;
        Color slotColor;

        ShopSlot()
            : x(0.0f)
            , y(0.0f)
            , size(0.0f)
            , typeContained(BASIC_TOWER)   // safe default
            , isRefreshButton(false)
            , slotColor{ 1.0f, 1.0f, 1.0f, 1.0f }
        {}
    };

    class Shop {
    public:
        void Init();
        void Update(std::vector<Tower>& activeTowers);
        void Draw();

        // Refreshes the tower types in the 5 slots
        void RefreshSlots();
        void Exit();

    private:
        static const int TOTAL_SLOTS = 6;
        static const int TOWER_SLOTS = 5;
        ShopSlot slots[TOTAL_SLOTS] = {};

        float slotSize = 60.0f;
        float padding = 60.0f;

        AEGfxVertexList* pMesh = nullptr;
    };
}