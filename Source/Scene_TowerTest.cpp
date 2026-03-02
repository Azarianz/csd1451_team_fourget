#include "Scene_TowerTest.h"
#include "AEEngine.h"
#include "math.h"

void Scene_TowerTest::Init()
{
    TowerHandler::TowerType tTypes[] = {TowerHandler::BASIC_TOWER, TowerHandler::SNIPER_TOWER, TowerHandler::SLOW_TOWER, TowerHandler::RAPID_TOWER};
    int numOfTowerTypes = sizeof(tTypes) / sizeof(tTypes[0]);

    int rndIndex = rand() % numOfTowerTypes;
    shopTower.ShopTowerInit(0, 0, 50, 50, tTypes[rndIndex]);

    // 1. Define the Path (A -> B -> C -> D)
    // Adjust these numbers to fit your screen resolution (1600x900)
    myPath.push_back({ -600.0f,  300.0f }); // Top Left
    myPath.push_back({ 600.0f,  300.0f }); // Top Right
    myPath.push_back({ 600.0f, -300.0f }); // Bottom Right
    myPath.push_back({ -600.0f, -300.0f }); // Bottom Left

}

void Scene_TowerTest::Update(float dt)
{
    for (auto& t : activeTowers) {
        if (t.details.fireTimer > 0.f) {
            t.details.fireTimer -= dt;
            if (t.details.fireTimer < 0.f) t.details.fireTimer = 0.f;
        }
    }

    AEInputGetCursorPosition(&mouseX, &mouseY);
    TowerHandler::UpdateTowerSystem((float)mouseX, (float)mouseY, shopTower, activeTowers);
    // Each tower shoots at most ONE enemy per frame
    for (auto& t : activeTowers) {
        for (auto* e : activeEnemies) {
            if (!e || e->health <= 0.0f) continue;
            TowerHandler::TowerShoot(t, *e, activeBullets);
            break; // one target per tower per frame (simple)
        }
    }

    TowerHandler::UpdateProjectiles(dt, activeEnemies, activeBullets);
    

    // INPUT: SPAWN ENEMIES
    // Press 1 for Zombie
    if (AEInputCheckTriggered(AEVK_C))
    {
        Zombie* z = new Zombie();
        z->Init();
        // Start at the first path point
        if (!myPath.empty()) {
            z->x = myPath[0].x;
            z->y = myPath[0].y;
        }
        activeEnemies.push_back(z);
    }

    // Press 2 for Skeleton
    if (AEInputCheckTriggered(AEVK_V))
    {
        Skeleton* s = new Skeleton();
        s->Init();
        if (!myPath.empty()) {
            s->x = myPath[0].x;
            s->y = myPath[0].y;
        }
        activeEnemies.push_back(s);
    }

    // Press 3 for Troll
    if (AEInputCheckTriggered(AEVK_B))
    {
        Troll* t = new Troll();
        t->Init();
        if (!myPath.empty()) {
            t->x = myPath[0].x;
            t->y = myPath[0].y;
        }
        activeEnemies.push_back(t);
    }

    // Press 3 for Golem
    if (AEInputCheckTriggered(AEVK_N))
    {
        Golem* t = new Golem();
        t->Init();
        if (!myPath.empty()) {
            t->x = myPath[0].x;
            t->y = myPath[0].y;
        }
        activeEnemies.push_back(t);
    }

    // Press 3 for Titan
    if (AEInputCheckTriggered(AEVK_M))
    {
        Titan* t = new Titan();
        t->Init();
        if (!myPath.empty()) {
            t->x = myPath[0].x;
            t->y = myPath[0].y;
        }
        activeEnemies.push_back(t);
    }
    // UPDATE ENEMIES
    for (auto* e : activeEnemies)
    {
        e->Update(dt, myPath);
    }

}

void Scene_TowerTest::Draw()
{
    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    shopTower.Draw();
    for (auto& t : activeTowers) {
        t.Draw();
    }

    for (auto& b : activeBullets) {
        b.Draw();
    }

    for (auto* e : activeEnemies){
        e->Draw();
    }
}

void Scene_TowerTest::Exit()
{
    for(auto & t : activeTowers) {
        t.Destroy();
	}
	activeTowers.clear();

    // Cleanup memory
    for (auto* e : activeEnemies) {
        e->Destroy();
        delete e;
    }
    activeEnemies.clear();
}
