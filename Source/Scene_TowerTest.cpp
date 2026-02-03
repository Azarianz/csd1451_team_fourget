#include "Scene_TowerTest.h"
#include "AEEngine.h"
#include "Tower.h"

void Scene_TowerTest::Init()
{
    shopTower.ShopTowerInit(0, 0, 50, 50, {1, 1, 1, 1});
}

void Scene_TowerTest::Update(float dt)
{
    AEInputGetCursorPosition(&mouseX, &mouseY);
    TowerHandler::UpdateTowerSystem((float)mouseX, (float)mouseY, shopTower, activeTowers);
}

void Scene_TowerTest::Draw()
{
    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    //shopTower.Draw();
    for (auto& t : activeTowers) {
        t.Draw();
    }
}

void Scene_TowerTest::Exit()
{
    
}
