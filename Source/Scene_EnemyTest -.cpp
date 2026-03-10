#include "Scene_EnemyTest.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Enemy::Init()
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    // 1. Define the Path (A -> B -> C -> D)
    // Adjust these numbers to fit your screen resolution (1600x900)
    myPath.push_back({ -600.0f,  300.0f }); // Top Left
    myPath.push_back({ 600.0f,  300.0f }); // Top Right
    myPath.push_back({ 600.0f, -300.0f }); // Bottom Right
    myPath.push_back({ -600.0f, -300.0f }); // Bottom Left

    // 2. Load the wave data
    if (!waveManager.LoadFromFile("Assets/waves.txt")) {
        PRINT("Failed to load waves.txt!\n");
    }
}

void Scene_Enemy::Update(float dt)
{

    // --- WAVE SPAWNING ---
    // Check if the wave manager wants to spawn an enemy this frame
    Enemy* newEnemy = waveManager.UpdateAndSpawn(dt, myPath);
    if (newEnemy) {
        activeEnemies.push_back(newEnemy);
    }

    // --- INPUT: SPAWN ENEMIES ---

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
    // --- UPDATE ENEMIES ---
    for (auto* e : activeEnemies)
    {
        e->Update(dt, myPath);
    }
}

void Scene_Enemy::DrawUI()
{
    if (m_uiFont < 0) return; // Don't draw if font failed to load

    char buf[64]; // Text buffer

    // --- 1. WAVE COUNTER ---
    if (waveManager.waveComplete) {
        sprintf_s(buf, "WAVES COMPLETE!");
    }
    else {
        sprintf_s(buf, "WAVE: %d / %d", waveManager.GetCurrentWaveNumber(), waveManager.GetTotalWaves());
    }

    // AEGfxPrint uses normalized coordinates (-1.0 to 1.0). Top-left is roughly x:-0.95, y:0.90
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 2. ENEMY COUNTER ---
    // Start with the enemies physically alive on the map
    int enemiesRemaining = (int)activeEnemies.size();

    // Add the enemies that haven't spawned yet in the current wave
    if (!waveManager.waveComplete) {
        int unspawnedEnemies = waveManager.GetTotalEnemiesInCurrentWave() - waveManager.spawnedInCurrentWave;
        enemiesRemaining += unspawnedEnemies;
    }

    sprintf_s(buf, "ENEMIES: %d", enemiesRemaining);
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.80f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
}


void Scene_Enemy::Draw()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);

    // Draw all enemies
    for (auto* e : activeEnemies)
    {
        e->Draw();
    }

    DrawUI();
}

void Scene_Enemy::Exit()
{
    // Cleanup memory
    // Clean up the font to prevent memory leaks
    if (m_uiFont >= 0) {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }
    for (auto* e : activeEnemies) {
        e->Destroy();
        delete e;
    }
    activeEnemies.clear();
}