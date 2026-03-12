#include "Scene_TowerTest.h"
#include "AEEngine.h"
#include "math.h"

// ============================================================
//  Init
// ============================================================
void Scene_TowerTest::Init()
{
    TowerHandler::LoadTowerAssets();

    // Default shop tower type (swap with H/J/K/L at runtime)
    shopTower.ShopTowerInit(0, 0, 50, 50, TowerHandler::BASIC_TOWER);

    // Enemy path: Top-Left -> Top-Right -> Bottom-Right -> Bottom-Left
    myPath.push_back({ -600.0f,  300.0f });
    myPath.push_back({ 600.0f,  300.0f });
    myPath.push_back({ 600.0f, -300.0f });
    myPath.push_back({ -600.0f, -300.0f });
}

void Scene_TowerTest::Update(float dt)
{
    // --------------------------------------------------------
    //  Tower fire-timer countdown
    // --------------------------------------------------------
    for (auto& t : activeTowers)
    {
        if (t.details.fireTimer > 0.f)
        {
            t.details.fireTimer -= dt;
            if (t.details.fireTimer < 0.f)
                t.details.fireTimer = 0.f;
        }
    }

    // --------------------------------------------------------
    //  Tower placement & dragging
    // --------------------------------------------------------
    AEInputGetCursorPosition(&mouseX, &mouseY);
    TowerHandler::UpdateTowerSystem((float)mouseX, (float)mouseY, shopTower, activeTowers);

    // --------------------------------------------------------
    //  Tower shooting (one target per tower per frame)
    // --------------------------------------------------------
    for (auto& t : activeTowers)
    {
        for (auto* e : activeEnemies)
        {
            if (!e || e->health <= 0.0f) continue;
            if (TowerHandler::TowerShoot(t, *e, activeBullets))
                break; // shot fired — move to next tower
        }
    }

    // --------------------------------------------------------
    //  Projectile movement & hit detection
    // --------------------------------------------------------
    TowerHandler::UpdateProjectiles(dt, activeEnemies, activeBullets);

    // INPUT: SPAWN ENEMIES
    // Press c for Zombie
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

    // Press v for Skeleton
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

    // Press b for Troll
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

    // Press n for Golem
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

    // Press m for Titan
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


    // --------------------------------------------------------
    //  Dead enemy cleanup
    // --------------------------------------------------------
    for (auto* e : activeEnemies)
    {
        if (e && e->health <= 0.0f)
        {
            // Prevent dangling bullet targets
            for (auto& b : activeBullets)
                if (b.target == e) b.target = nullptr;

            e->Destroy();
            delete e;
        }
    }
    activeEnemies.erase(
        std::remove_if(activeEnemies.begin(), activeEnemies.end(),
            [](Enemy* e) { return !e || e->health <= 0.0f; }),
        activeEnemies.end());


   // --------------------------------------------------------
   //  Tower level up  (U = level up selected tower)
   // --------------------------------------------------------
    if (AEInputCheckTriggered(AEVK_U))
    {
        for (auto& t : activeTowers)
        {
            if (t.isSelected)
            {
                t.LevelUp(); // returns false if already max — no action needed
                break;
            }
        }
    }

    // --------------------------------------------------------
    //  Shop tower type swap  (H/J/K/L)
    // --------------------------------------------------------
    if (AEInputCheckTriggered(AEVK_H)) shopTower.SetType(TowerHandler::BASIC_TOWER);
    if (AEInputCheckTriggered(AEVK_J)) shopTower.SetType(TowerHandler::SNIPER_TOWER);
    if (AEInputCheckTriggered(AEVK_K)) shopTower.SetType(TowerHandler::SLOW_TOWER);
    if (AEInputCheckTriggered(AEVK_L)) shopTower.SetType(TowerHandler::RAPID_TOWER);
	
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

    Graphics::RenderAll();
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

	Graphics::Shutdown();
}
