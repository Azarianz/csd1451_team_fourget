#include "Scene_ShopTest.h"
#include "AEEngine.h"
#include "Shop.h"
#include "Utility.h"

void Scene_ShopTest::Init() {
    shop.Init();
}

void Scene_ShopTest::Update(float dt) {
    float mX, mY;
    Utility::GetWorldMousePos(mX, mY);

    // Update the shop logic (spawns new towers into the vector)
    shop.Update(activeTowers);

    // Update existing towers logic
    // Created a dummy ShopTower to satisfy the function signature
    TowerHandler::ShopTower dummyShop;
    TowerHandler::UpdateTowerSystem(mX, mY, dummyShop, activeTowers);
}

void Scene_ShopTest::Draw() {
    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);
    // Draw towers
    for (auto& t : activeTowers) {
        t.Draw();
    }

    // Draw the shop UI
    shop.Draw();
}

void Scene_ShopTest::Exit() {
    shop.Exit();
    activeTowers.clear();
}