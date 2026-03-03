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
    };

    class Shop {
    public:
        void Init();
        void Update(std::vector<Tower>& activeTowers);
        void Draw();

        // Refreshes the tower types in the 5 slots
        void RefreshSlots();
        void Exit();

        // New helper to manage points
        void AddPoints(int amount) { m_points += amount; }

    private:
        int m_points = 100;         // Starting points
        s8 m_uiFont = -1;           // Font handle
        const int TOWER_COST = 25;  // Standard purchase price

        static const int TOTAL_SLOTS = 6;
        static const int TOWER_SLOTS = 5;
        ShopSlot slots[TOTAL_SLOTS];

        float slotSize = 60.0f;
        float padding = 60.0f;
        AEGfxVertexList* pMesh = nullptr;

        // Helper function for rendering the UI
        void DrawPoints();
    };
}