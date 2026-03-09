#include "Scene_ShopTest.h"
#include "AEEngine.h"
#include "Shop.h"
#include "Utility.h"
#include <algorithm>

void Scene_ShopTest::Init()
{
    TowerHandler::LoadTowerAssets();
    shop.Init();

    // Define enemy path
    myPath.push_back({ -600.0f,  300.0f }); // Top Left
    myPath.push_back({ 600.0f,  300.0f }); // Top Right
    myPath.push_back({ 600.0f, -300.0f }); // Bottom Right
    myPath.push_back({ -600.0f, -300.0f }); // Bottom Left
}

void Scene_ShopTest::Update(float dt)
{
    // Fire timer countdown
    for (auto& t : activeTowers) {
        if (t.details.fireTimer > 0.f) {
            t.details.fireTimer -= dt;
            if (t.details.fireTimer < 0.f) t.details.fireTimer = 0.f;
        }
    }

    // Shop: buy and place towers 
    shop.Update(activeTowers);

    // Tower dragging (dummy shop — shop already handles placement) 
    TowerHandler::ShopTower dummyShop;
    float mX, mY;
    Utility::GetWorldMousePos(mX, mY);
    TowerHandler::UpdateTowerSystem(mX, mY, dummyShop, activeTowers);

    // Tower shooting: each tower shoots at most one enemy per frame
    for (auto& t : activeTowers) {
        for (auto* e : activeEnemies) {
            if (!e || e->health <= 0.0f) continue;
            if (TowerHandler::TowerShoot(t, *e, activeBullets))
                break;
        }
    }

    // Update bullets 
    TowerHandler::UpdateProjectiles(dt, activeEnemies, activeBullets);

    // Spawn enemies (C=Zombie, V=Skeleton, B=Troll, N=Golem, M=Titan)
    if (AEInputCheckTriggered(AEVK_C)) {
        Zombie* z = new Zombie(); z->Init();
        if (!myPath.empty()) { z->x = myPath[0].x; z->y = myPath[0].y; }
        activeEnemies.push_back(z);
    }
    if (AEInputCheckTriggered(AEVK_V)) {
        Skeleton* s = new Skeleton(); s->Init();
        if (!myPath.empty()) { s->x = myPath[0].x; s->y = myPath[0].y; }
        activeEnemies.push_back(s);
    }
    if (AEInputCheckTriggered(AEVK_B)) {
        Troll* t = new Troll(); t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }
    if (AEInputCheckTriggered(AEVK_N)) {
        Golem* g = new Golem(); g->Init();
        if (!myPath.empty()) { g->x = myPath[0].x; g->y = myPath[0].y; }
        activeEnemies.push_back(g);
    }
    if (AEInputCheckTriggered(AEVK_M)) {
        Titan* t = new Titan(); t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }

    // Update enemies along path
    for (auto* e : activeEnemies)
        e->Update(dt, myPath);

    // Remove dead enemies (null bullet targets first)
    for (auto* e : activeEnemies) {
        if (e && e->health <= 0.0f) {
            for (auto& b : activeBullets)
                if (b.target == e) b.target = nullptr;
            shop.AddPoints(5);
            e->Destroy();
            delete e;
            e = nullptr;
        }
    }
    activeEnemies.erase(
        std::remove_if(activeEnemies.begin(), activeEnemies.end(),
            [](Enemy* e) { return !e || e->health <= 0.0f; }),
        activeEnemies.end());

    // Level up selected tower (U key)
    if (AEInputCheckTriggered(AEVK_U)) {
        for (auto& t : activeTowers) {
            if (t.isSelected) { 
                int cost = (t.details.level == 1) ? 50 : 100;
                if (shop.SpendPoints(cost)) {
                    t.LevelUp(); 
                break;
                } 
            }
        }
    }
}

void Scene_ShopTest::Draw()
{
    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    // Draw tower range circles + base circles
    for (auto& t : activeTowers)
        t.Draw();

    // Draw bullets
    for (auto& b : activeBullets)
        b.Draw();

    // Draw enemies
    for (auto* e : activeEnemies)
        e->Draw();

    // Draw shop UI
    shop.Draw();

    // Render all Graphics sprites (tower sprites)
    Graphics::RenderAll();
}

void Scene_ShopTest::Exit()
{
    shop.Exit();

    for (auto& t : activeTowers)
        t.Destroy();
    activeTowers.clear();

    for (auto* e : activeEnemies) {
        e->Destroy();
        delete e;
    }
    activeEnemies.clear();

    Graphics::Shutdown();
}