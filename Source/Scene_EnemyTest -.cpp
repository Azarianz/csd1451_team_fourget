#include "Scene_EnemyTest.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Enemy::Init()
{
    // 1. Define the Path (A -> B -> C -> D)
    // Adjust these numbers to fit your screen resolution (1600x900)
    myPath.push_back({ -600.0f,  300.0f }); // Top Left
    myPath.push_back({ 600.0f,  300.0f }); // Top Right
    myPath.push_back({ 600.0f, -300.0f }); // Bottom Right
    myPath.push_back({ -600.0f, -300.0f }); // Bottom Left
}

void Scene_Enemy::Update(float dt)
{
    // --- INPUT: SPAWN ENEMIES ---

    // Press 1 for Zombie
    if (AEInputCheckTriggered(AEVK_7))
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
    if (AEInputCheckTriggered(AEVK_8))
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
    if (AEInputCheckTriggered(AEVK_9))
    {
        Troll* t = new Troll();
        t->Init();
        if (!myPath.empty()) {
            t->x = myPath[0].x;
            t->y = myPath[0].y;
        }
        activeEnemies.push_back(t);
    }

    // --- UPDATE ENEMIES ---
    for (auto* e : activeEnemies)
    {
        e->Update(dt, myPath);
    }
}

void Scene_Enemy::Draw()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);

    // Draw all enemies
    for (auto* e : activeEnemies)
    {
        e->Draw();
    }
}

void Scene_Enemy::Exit()
{
    // Cleanup memory
    for (auto* e : activeEnemies) {
        e->Destroy();
        delete e;
    }
    activeEnemies.clear();
}