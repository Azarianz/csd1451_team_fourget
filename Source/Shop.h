#pragma once
#include "AEEngine.h"
#include "Tower.h"

struct ShopItem {
    int towerID;
    int cost;
    Color color;
};

struct ShopSlot {
    AEGfxTexture* towerArt = nullptr;       // Tower 
    AEGfxTexture* numArt = nullptr;    // Number
    ShopItem item;
    float x, y, width, height;

    bool isMouseOver(float mx, float my) const;
};

struct Shop {
    // 5 Tower slots + 1 Refresh slot
    ShopSlot shopListing[5];
    ShopSlot refreshButton;

    // We now pass a pointer to the main tower list to manage spawning
    void Init();
    // Returns true if a tower was bought, or if the shop was refreshed
    TowerHandler::Tower* Update(int& playerCurrency);
    void Draw();
    void Unload();
};