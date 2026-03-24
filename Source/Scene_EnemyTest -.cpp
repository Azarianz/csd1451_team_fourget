// Scene_EnemyTest.cpp
#include "Scene_EnemyTest.h"
#include "AEEngine.h"
#include "AEInput.h"
#include <cstdio> 

// Helper to generate the mesh for the flag UI
static AEGfxVertexList* CreateFlagMesh(int row, int col)
{
    int SHEET_COLS = 18;
    int SHEET_ROWS = 11;
    int ty = (SHEET_ROWS - 1) - row;
    float u0 = (float)col / (float)SHEET_COLS;
    float u1 = (float)(col + 1) / (float)SHEET_COLS;
    float v0 = (float)ty / (float)SHEET_ROWS;
    float v1 = (float)(ty + 1) / (float)SHEET_ROWS;

    AEGfxMeshStart();
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, -0.5f, 0xFFFFFFFF, u1, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0
    );
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, u0, v1,
        0.5f, 0.5f, 0xFFFFFFFF, u1, v0,
        -0.5f, 0.5f, 0xFFFFFFFF, u0, v0
    );
    return AEGfxMeshEnd();
}

void Scene_Enemy::Init()
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    myPath.push_back({ -600.0f,  300.0f });
    myPath.push_back({ 600.0f,  300.0f });
    myPath.push_back({ 600.0f, -300.0f });
    myPath.push_back({ -600.0f, -300.0f });

    if (!waveManager.LoadFromFile("Assets/waves.txt")) {
        PRINT("Failed to load waves.txt!\n");
    }

    gameSpeedMultiplier = 1.0f;

    // Load sprite sheet and specific flag UV meshes (Col 12, Rows 0, 1, 2)
    m_spriteSheet = AEGfxTextureLoad("Assets/rawspritesheet.png");
    m_flagMeshes[0] = CreateFlagMesh(9, 17); // White Flag (1x)
    m_flagMeshes[1] = CreateFlagMesh(6, 17); // Green Flag (2x)
    m_flagMeshes[2] = CreateFlagMesh(7, 17); // Blue Flag (4x)
}

void Scene_Enemy::Update(float dt)
{
    if (AEInputCheckTriggered(AEVK_SPACE))
    {
        if (gameSpeedMultiplier == 1.0f) gameSpeedMultiplier = 2.0f;
        else if (gameSpeedMultiplier == 2.0f) gameSpeedMultiplier = 2.0f;
        else gameSpeedMultiplier = 1.0f;
    }

    float scaled_dt = dt * gameSpeedMultiplier;

    Enemy* newEnemy = waveManager.UpdateAndSpawn(scaled_dt, myPath);
    if (newEnemy) {
        activeEnemies.push_back(newEnemy);
    }

    if (AEInputCheckTriggered(AEVK_C))
    {
        Zombie* z = new Zombie();
        z->Init();
        if (!myPath.empty()) { z->x = myPath[0].x; z->y = myPath[0].y; }
        activeEnemies.push_back(z);
    }
    if (AEInputCheckTriggered(AEVK_V))
    {
        Skeleton* s = new Skeleton();
        s->Init();
        if (!myPath.empty()) { s->x = myPath[0].x; s->y = myPath[0].y; }
        activeEnemies.push_back(s);
    }
    if (AEInputCheckTriggered(AEVK_B))
    {
        Troll* t = new Troll();
        t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }
    if (AEInputCheckTriggered(AEVK_N))
    {
        Golem* t = new Golem();
        t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }
    if (AEInputCheckTriggered(AEVK_M))
    {
        Titan* t = new Titan();
        t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }
    if (AEInputCheckTriggered(AEVK_X))
    {
        wavestarter* t = new wavestarter();
        t->Init();
        if (!myPath.empty()) { t->x = myPath[0].x; t->y = myPath[0].y; }
        activeEnemies.push_back(t);
    }

    for (auto* e : activeEnemies)
    {
        e->Update(scaled_dt, myPath);
    }
}

void Scene_Enemy::DrawUI()
{
    if (m_uiFont < 0) return;

    char buf[64];

    // --- 1. WAVE COUNTER ---
    if (waveManager.waveComplete) {
        sprintf_s(buf, "WAVES COMPLETE!");
    }
    else {
        sprintf_s(buf, "WAVE: %d / %d", waveManager.GetWavestarterCount(), waveManager.GetTotalWavestarters());
    }
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 2. ENEMY COUNTER ---
    int enemiesRemaining = (int)activeEnemies.size();

    if (!waveManager.waveComplete) {
        int unspawnedEnemies = waveManager.GetTotalEnemiesInCurrentWave() - waveManager.spawnedInCurrentWave;
        enemiesRemaining += unspawnedEnemies;
    }

    sprintf_s(buf, "ENEMIES: %d", enemiesRemaining);
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.80f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 3. WAVE TIMER ---
    if (!waveManager.waveComplete && waveManager.IsWaitingForWavestarter()) {
        sprintf_s(buf, "NEXT WAVE IN: %.1f", waveManager.GetTimeUntilNextSpawn());
        AEGfxPrint(m_uiFont, buf, -0.2f, 0.90f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    }

    // --- 4. GAME SPEED INDICATOR (FLAG) ---
    sprintf_s(buf, "SPEED: ");
    AEGfxPrint(m_uiFont, buf, 0.65f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    int flagIndex = 0; // White Flag (1.0x)
    if (gameSpeedMultiplier == 2.0f) flagIndex = 1; // Green Flag (2.0x)
    else if (gameSpeedMultiplier == 4.0f) flagIndex = 2; // Blue Flag (4.0x)

    // Render the selected flag mesh
    if (m_spriteSheet && m_flagMeshes[flagIndex])
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(m_spriteSheet, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

        AEMtx33 scaleM, rotM, transM, transform;
        AEMtx33Scale(&scaleM, 40.0f, 40.0f); // Size of the flag
        AEMtx33Rot(&rotM, 0.0f);
        // Adjust these X, Y values if the flag does not sit perfectly next to the "SPEED:" text
        AEMtx33Trans(&transM, 730.0f, 415.0f);

        AEMtx33Concat(&transform, &rotM, &scaleM);
        AEMtx33Concat(&transform, &transM, &transform);

        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(m_flagMeshes[flagIndex], AE_GFX_MDM_TRIANGLES);
    }
}

void Scene_Enemy::Draw()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);

    for (auto* e : activeEnemies)
    {
        e->Draw();
    }

    DrawUI();
}

void Scene_Enemy::Exit()
{
    if (m_uiFont >= 0) {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    for (auto* e : activeEnemies) {
        e->Destroy();
        delete e;
    }
    activeEnemies.clear();

    // Clean up textures and meshes
    if (m_spriteSheet) {
        AEGfxTextureUnload(m_spriteSheet);
        m_spriteSheet = nullptr;
    }

    for (int i = 0; i < 3; ++i) {
        if (m_flagMeshes[i]) {
            AEGfxMeshFree(m_flagMeshes[i]);
            m_flagMeshes[i] = nullptr;
        }
    }
}