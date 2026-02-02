#include "Shop.h"
#include "Utility.h"
#include "AEInput.h"
#include "Tower.h"
#include <string>

bool ShopSlot::isMouseOver(float mx, float my) const {
    return (mx >= x - width / 2.0f && mx <= x + width / 2.0f &&
        my >= y - height / 2.0f && my <= y + height / 2.0f);
}

static void DrawUIQuad(float cx, float cy, float w, float h) {
    AEGfxMeshStart();
    unsigned int col = 0xFFFFFFFF;
    AEGfxTriAdd(cx - w / 2, cy - h / 2, col, 0, 1, cx + w / 2, cy - h / 2, col, 1, 1, cx + w / 2, cy + h / 2, col, 1, 0);
    AEGfxTriAdd(cx - w / 2, cy - h / 2, col, 0, 1, cx + w / 2, cy + h / 2, col, 1, 0, cx - w / 2, cy + h / 2, col, 0, 0);
    AEGfxVertexList* quad = AEGfxMeshEnd();

    AEMtx33 id;
    AEMtx33Identity(&id);
    AEGfxSetTransform(id.m);
    AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);
    AEGfxMeshFree(quad);
}

static void DrawTexturedQuad(float cx, float cy, float w, float h, AEGfxTexture* tex) {
    if (!tex) return;
    AEGfxTextureSet(tex, 0, 0);

    AEGfxMeshStart();
    unsigned int col = 0xFFFFFFFF;
    AEGfxTriAdd(cx - w / 2, cy - h / 2, col, 0, 1, cx + w / 2, cy - h / 2, col, 1, 1, cx + w / 2, cy + h / 2, col, 1, 0);
    AEGfxTriAdd(cx - w / 2, cy - h / 2, col, 0, 1, cx + w / 2, cy + h / 2, col, 1, 0, cx - w / 2, cy + h / 2, col, 0, 0);
    AEGfxVertexList* quad = AEGfxMeshEnd();

    AEMtx33 id;
    AEMtx33Identity(&id);
    AEGfxSetTransform(id.m);
    AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);
    AEGfxMeshFree(quad);
}

void Shop::Init() {
    float slotW = 70.0f; 
    float slotH = 70.0f;
    //float spacing = 10.0f;
    float totalW = (6 * slotW); //+ (5 * spacing);
    float startX = -(totalW / 2.0f) + (slotW / 2.0f);
    float posY = -415.0f;

    // Load Tower Assets 1-5
    for (int i = 0; i < 5; ++i) {
        //shopListing[i].x = startX + i * (slotW + spacing);
        shopListing[i].x = startX + i * slotW;
        shopListing[i].y = posY;
        shopListing[i].width = slotW;
        shopListing[i].height = slotH;
        shopListing[i].item.towerID = i + 1;
        shopListing[i].item.cost = (i + 1) * 50;

        std::string pathNumber = "Assets/tile_" + std::to_string(i + 1) + ".png";
        std::string pathTower = "Assets/tower_" + std::to_string(i + 1) + ".png";
        shopListing[i].towerArt = AEGfxTextureLoad(pathTower.c_str());
        shopListing[i].numArt = AEGfxTextureLoad(pathNumber.c_str());
    }

    // Refresh Button (Slot 6)
    //refreshButton.x = startX + 5 * (slotW + spacing);
    refreshButton.x = startX + 5 * slotW;
    refreshButton.y = posY;
    refreshButton.width = slotW;
    refreshButton.height = slotH;
    refreshButton.numArt = AEGfxTextureLoad("Assets/refresh.png");
}

TowerHandler::Tower* Shop::Update(int& playerCurrency) {
    if (AEInputCheckTriggered(AEVK_LBUTTON)) {
        float mX, mY;
        Utility::GetWorldMousePos(mX, mY);

        // Check Tower purchases
        for (int i = 0; i < 5; ++i) {
            if (shopListing[i].isMouseOver(mX, mY) && playerCurrency >= shopListing[i].item.cost) {
                playerCurrency -= shopListing[i].item.cost;

                // Use factory to create tower and set it to dragging mode
                TowerHandler::Tower temp;
                TowerHandler::Tower* newTower = temp.CreateTower(mX, mY, shopListing[i].item.towerID);
                newTower->isDragging = true;
                return newTower;
            }
        }

        // Check Refresh
        if (refreshButton.isMouseOver(mX, mY) && playerCurrency >= 10) {
            playerCurrency -= 10;
            // Add randomization logic here if desired
        }
    }
    return nullptr;
}

void Shop::Draw() {
    // Draw Background Boxes
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // Draw 5 tower slot backgrounds
    for (int i = 0; i < 5; ++i) {
        AEGfxSetColorToMultiply(0.15f, 0.15f, 0.15f, 1.0f);
        DrawUIQuad(shopListing[i].x, shopListing[i].y, shopListing[i].width, shopListing[i].height);
    }
    // Draw Refresh button background separately
    AEGfxSetColorToMultiply(0.15f, 0.15f, 0.15f, 1.0f);
    DrawUIQuad(refreshButton.x, refreshButton.y, refreshButton.width, refreshButton.height);

    // Draw Textures
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(1, 1, 1, 1);

    for (int i = 0; i < 5; ++i) {
        // The Tower Asset
        if (shopListing[i].towerArt) {
            float towerSize = 50.0f; 
            DrawTexturedQuad(shopListing[i].x, shopListing[i].y, towerSize, towerSize, shopListing[i].towerArt);
        }

        // The Number Icon
        if (shopListing[i].numArt) {
            float iconSize = 40.0f; 
            // Calculate bottom-right offset
            float iconX = shopListing[i].x + (shopListing[i].width / 2.0f) - (iconSize / 2.0f) - 2.0f;
            float iconY = shopListing[i].y - (shopListing[i].height / 2.0f) + (iconSize / 2.0f) + 2.0f;

            DrawTexturedQuad(iconX, iconY, iconSize, iconSize, shopListing[i].numArt);
        }
    }

    // Draw Refresh Icon
    if (refreshButton.numArt) {
        DrawTexturedQuad(refreshButton.x, refreshButton.y, refreshButton.width - 10.0f, refreshButton.height - 10.0f, refreshButton.numArt);
    }
}

void Shop::Unload() {
    for (int i = 0; i < 5; ++i) {
        if (shopListing[i].towerArt) {
            AEGfxTextureUnload(shopListing[i].towerArt);
            shopListing[i].towerArt = nullptr;
        }
        if (shopListing[i].numArt) {
            AEGfxTextureUnload(shopListing[i].numArt);
            shopListing[i].numArt = nullptr;
        }
    }

    if (refreshButton.numArt) {
        AEGfxTextureUnload(refreshButton.numArt);
        refreshButton.numArt = nullptr;
    }
}