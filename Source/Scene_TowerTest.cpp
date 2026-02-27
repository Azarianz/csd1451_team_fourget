#include "Scene_TowerTest.h"
#include "AEEngine.h"
#include "math.h"
#include "Tower.h"

void Scene_TowerTest::Init()
{
    TowerHandler::TowerType tTypes[] = {TowerHandler::BASIC_TOWER, TowerHandler::SNIPER_TOWER, TowerHandler::SLOW_TOWER, TowerHandler::RAPID_TOWER};
    int numOfTowerTypes = sizeof(tTypes) / sizeof(tTypes[0]);

    int rndIndex = rand() % numOfTowerTypes;
    shopTower.ShopTowerInit(0, 0, 50, 50, tTypes[rndIndex]);
}

void Scene_TowerTest::Update(float dt)
{
    AEInputGetCursorPosition(&mouseX, &mouseY);
    TowerHandler::UpdateTowerSystem((float)mouseX, (float)mouseY, shopTower, activeTowers);
}

void Scene_TowerTest::Draw()
{
    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    shopTower.Draw();
    for (auto& t : activeTowers) {
        t.Draw();
    }
}

void Scene_TowerTest::Exit()
{
    
}
