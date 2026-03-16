#include "Scene_Prototype.h"

#include "AEEngine.h"
#include "AEInput.h"
#include "Utility.h"

#pragma region Helper Funcs
// --------------------------------------------------------
//  DestroyGrid
//  Safely destroys the current grid instance and clears pointer.
// --------------------------------------------------------
void Scene_Prototype::DestroyGrid()
{
    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
}

// --------------------------------------------------------
//  InitLevelAndGrid
//  Loads the level data, creates the grid system, and builds
//  the enemy path used for navigation.
// --------------------------------------------------------
bool Scene_Prototype::InitLevelAndGrid()
{
    //Load Level File
    levelFile = GameSettings::selectedLevelFile;

    PRINT("Prototype received level file: %s\n", levelFile.c_str());

    if (levelFile.empty())
    {
        PRINT("Level file empty, defaulting to ../../Assets/Levels/level_01.txt\n");
        levelFile = "../../Assets/Levels/level_01.txt";
    }

    PRINT("Loading level file: %s\n", levelFile.c_str());

    if (!level.Init(levelFile.c_str()))
    {
        PRINT("FAILED to load level: %s\n", levelFile.c_str());
        return false;
    }

    occupied.assign((size_t)level.width * (size_t)level.height, 0);

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    path.clear();
    if (!level.BuildPath(*grid, path))
    {
        PRINT("FAILED to build enemy path from region flags.\n");
        return false;
    }

    return true;
}

// --------------------------------------------------------
//  InitAudio
//  Loads and starts background music for the scene.
// --------------------------------------------------------
void Scene_Prototype::InitAudio()
{
    m_bgm = AEAudioLoadMusic("Assets/bouken.mp3");
    m_bgmGroup = AEAudioCreateGroup();
    m_bgmLoaded = true;
    AEAudioPlay(m_bgm, m_bgmGroup, 1.0f, 1.0f, -1);

    float vol = GameSettings::masterVolume / 100.0f;
    AEAudioSetGroupVolume(m_bgmGroup, vol);
}

// --------------------------------------------------------
//  DrawUI
//  Renders gameplay UI information.
//  - Displays current wave progress
//  - Displays active enemy count
//  - Draws pause/resume label in the top-right
// --------------------------------------------------------
void Scene_Prototype::DrawUI()
{
    if (m_uiFont < 0) return;

    char buf[64];

    if (waveManager.waveComplete)
        sprintf_s(buf, "WAVES COMPLETE!");
    else
        sprintf_s(buf, "WAVE: %d / %d", waveManager.GetCurrentWaveNumber(), waveManager.GetTotalWaves());

    AEGfxPrint(m_uiFont, buf, -0.95f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    sprintf_s(buf, "ENEMIES: %d", (int)enemies.size());
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.80f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    const char* pauseLabel = m_paused ? "RESUME" : "PAUSE";
    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();
    float pauseScreenX = screenW * 0.88f;
    float pauseScreenY = screenH * 0.08f;

    AEGfxPrint(m_uiFont, pauseLabel,
        (pauseScreenX / screenW) * 2.0f - 1.0f,
        1.0f - (pauseScreenY / screenH) * 2.0f,
        1.0f, 1.0f, 1.0f, 0.2f, 1.0f);
}

// --------------------------------------------------------
//  IsPauseButtonClicked
//  Checks whether the mouse click is inside the pause button.
//  - Calculates the UI text position
//  - Builds a hitbox around the pause label
//  - Returns true if the cursor is inside the hit area
// --------------------------------------------------------
bool Scene_Prototype::IsPauseButtonClicked(int mouseX, int mouseY) const
{
    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();

    float textX = screenW * 0.88f;
    float textY = screenH * 0.08f;

    float hitW = screenW * 0.10f;
    float hitH = screenH * 0.06f;

    float top = textY - hitH;
    float bottom = textY;

    return ((float)mouseX >= textX && (float)mouseX <= textX + hitW &&
        (float)mouseY >= top && (float)mouseY <= bottom);
}

// --------------------------------------------------------
//  ResetRuntimeState
//  Clears all gameplay containers and resets runtime flags.
//  - Clears towers, bullets and enemies
//  - Resets mouse state
//  - Clears game over and pause flags
// --------------------------------------------------------
void Scene_Prototype::ResetRuntimeState()
{
    for (Enemy* e : enemies)
        delete e;
    enemies.clear();

    activeTowers.clear();
    activeBullets.clear();

    wasLmbDown = false;
    baseTowerIndex = -1;
    gameOver = false;
    m_paused = false;
}

// --------------------------------------------------------
//  HandleUserInputs
//  Processes player input for pause, debug upgrade, and
//  tower selection / dragging interactions.
// --------------------------------------------------------
void Scene_Prototype::HandleUserInputs(float worldX, float worldY, int mouseX, int mouseY)
{
    bool toggledPause = false;

    if (AEInputCheckTriggered(AEVK_P))
        toggledPause = true;

    if (AEInputCheckTriggered(AEVK_LBUTTON) && IsPauseButtonClicked(mouseX, mouseY))
        toggledPause = true;

    if (toggledPause)
    {
        m_paused = !m_paused;

        if (m_bgmLoaded)
        {
            if (m_paused) AEAudioPauseGroup(m_bgmGroup);
            else          AEAudioResumeGroup(m_bgmGroup);
        }
    }

    if (m_paused)
        return;

    // Debug Upgrade tower
    if (AEInputCheckTriggered(AEVK_U))
    {
        for (auto& t : activeTowers)
        {
            if (t.isSelected && !t.isDragging)
            {
                t.LevelUp();
                break;
            }
        }
    }

    UpdateSelectionAndDragging(worldX, worldY, mouseX, mouseY);
}

// --------------------------------------------------------
//  UpdateSelectionAndDragging
//  Handles mouse interactions with towers.
//  - Updates shop UI interactions
//  - Processes tower dragging and placement
//  - Selects the topmost tower on click
// --------------------------------------------------------
void Scene_Prototype::UpdateSelectionAndDragging(float worldX, float worldY, int mouseX, int mouseY)
{
    bool lmbDown = AEInputCheckCurr(AEVK_LBUTTON);
    bool justPressedLmb = (!wasLmbDown && lmbDown);
    bool justReleasedLmb = (wasLmbDown && !lmbDown);

    shop.Update(activeTowers);

    buildMergeSystem.UpdateDragging(worldX, worldY, lmbDown, justReleasedLmb, mouseX, mouseY);

    if (justPressedLmb && !buildMergeSystem.IsDraggingTower())
        TowerHandler::SelectTopmostTower(worldX, worldY, activeTowers);

    wasLmbDown = lmbDown;
}

// --------------------------------------------------------
//  UpdateEnemies
//  Handles enemy spawning from the wave manager and updates
//  all active enemies along the path.
// --------------------------------------------------------
void Scene_Prototype::UpdateEnemies(float dt)
{
    Enemy* spawnedEnemy = waveManager.UpdateAndSpawn(dt, path);
    if (spawnedEnemy)
        enemies.push_back(spawnedEnemy);

    for (Enemy* e : enemies)
    {
        if (e)
            e->Update(dt, path);
    }
}

// --------------------------------------------------------
//  UpdateBaseCollision
//  Handles collisions between enemies and the base tower.
//  - Enemies reaching the base are marked dead/escaped
//  - Base takes contact damage
//  - Triggers game over if base health reaches zero
// --------------------------------------------------------
void Scene_Prototype::UpdateBaseCollision()
{
    if (baseTowerIndex < 0 || baseTowerIndex >= (int)activeTowers.size())
        return;

    TowerHandler::Tower& base = activeTowers[(size_t)baseTowerIndex];

    for (Enemy* e : enemies)
    {
        if (!e || e->health <= 0.0f)
            continue;

        if (TowerHandler::CircleCircleCollision(
            base.x, base.y, base._sizeX * 0.5f,
            e->x, e->y, e->_sizeX * 0.5f) ||
            e->reachedEnd)
        {
            e->health = 0.0f;
            e->escapedBase = true;

            if (base.TakeDamage(base.details.contactDamage))
            {
                gameOver = true;
                if (m_bgmLoaded)
                    AEAudioPauseGroup(m_bgmGroup);
            }
        }
    }
}

// --------------------------------------------------------
//  UpdateReturningTowers
//  Handles towers being returned to the shop.
//  - Moves towers toward their return target
//  - Refunds cost when they reach the shop slot
//  - Removes the tower from the active list
// --------------------------------------------------------
void Scene_Prototype::UpdateReturningTowers(float dt)
{
    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
    {
        TowerHandler::Tower& t = activeTowers[(size_t)i];
        if (!t.isReturning)
            continue;

        const float lerpSpeed = 8.0f;
        t.x += (t.returnTargetX - t.x) * lerpSpeed * dt;
        t.y += (t.returnTargetY - t.y) * lerpSpeed * dt;

        float dx = t.returnTargetX - t.x;
        float dy = t.returnTargetY - t.y;
        if (dx * dx + dy * dy < 4.0f)
        {
            shop.AddPoints(shop.GetTowerCost());
            shop.RestoreSlot(t.sourceSlotIndex);
            buildMergeSystem.RemoveTowerAtIndex(i);
            buildMergeSystem.RebuildOccupiedFromTowers();
        }
    }
}

// --------------------------------------------------------
//  CleanupDeadEnemies
//  Removes defeated enemies from the scene.
//  - Awards points for enemies killed normally
//  - Does not reward enemies that escaped to the base
// --------------------------------------------------------
void Scene_Prototype::CleanupDeadEnemies()
{
    for (int i = (int)enemies.size() - 1; i >= 0; --i)
    {
        Enemy* e = enemies[(size_t)i];
        if (!e || e->health <= 0.0f)
        {
            if (e && !e->escapedBase)
                shop.AddPoints(e->GetPoints());

            delete e;
            enemies.erase(enemies.begin() + i);
        }
    }
}

// --------------------------------------------------------
//  CreateBaseTower
//  Spawns the base tower at the end of the enemy path.
// --------------------------------------------------------
void Scene_Prototype::CreateBaseTower()
{
    if (!path.empty())
    {
        baseTowerIndex = TowerHandler::AddBaseTower(
            activeTowers,
            path.back().x,
            path.back().y,
            80.0f,
            80.0f
        );

        buildMergeSystem.RebuildOccupiedFromTowers();
    }
}
#pragma endregion

#pragma region Scene Funcs
void Scene_Prototype::Init()
{
    DestroyGrid();

    if (!InitLevelAndGrid())
    {
        PRINT("Scene_Prototype Init failed to load level.\n");
        return;
    }

    ResetRuntimeState();

    TowerHandler::LoadTowerAssets();

    shop.Init();
    buildMergeSystem.Init(&level, grid, &shop, &activeTowers, &activeBullets, &occupied);

    InitAudio();
	CreateBaseTower();

    if (gameOverFont < 0)
        gameOverFont = AEGfxCreateFont("Assets/buggy-font.ttf", 64);

    if (m_uiFont < 0)
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    if (!waveManager.LoadFromFile("Assets/waves.txt"))
        PRINT("Failed to load waves.txt!\n");
}

void Scene_Prototype::Update(float dt)
{
    if (gameOver) return;
    if (!grid) return;

    int mouseX = 0, mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float worldX = 0.0f, worldY = 0.0f;
    Utility::GetWorldMousePos(worldX, worldY);

    HandleUserInputs(worldX, worldY, mouseX, mouseY);
    if (m_paused)
        return;

    UpdateEnemies(dt);
    TowerHandler::UpdateTowerLogic(dt, activeTowers, enemies, activeBullets);
    UpdateBaseCollision();
    CleanupDeadEnemies();
    UpdateReturningTowers(dt);
}

void Scene_Prototype::Draw()
{
    if (!grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    level.Draw(*grid, 1.0f);

    for (auto& t : activeTowers)
        t.Draw();

    for (auto& b : activeBullets)
        b.Draw();

    for (Enemy* e : enemies)
    {
        if (e)
            e->Draw();
    }

    if (buildMergeSystem.IsDraggingTower())
        buildMergeSystem.DrawOverlay();

    shop.Draw();
    DrawUI();

    Graphics::RenderAll();

    if (gameOver && gameOverFont >= 0)
    {
        AEGfxPrint(gameOverFont, "GAME OVER",
            -0.4f, 0.0f,
            1.2f,
            1.0f, 0.1f, 0.1f,
            1.0f);
    }
}

void Scene_Prototype::Exit()
{
    shop.Exit();

    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
        buildMergeSystem.RemoveTowerAtIndex(i);
    activeTowers.clear();

    for (Enemy* e : enemies)
        delete e;
    enemies.clear();

    if (gameOverFont >= 0)
    {
        AEGfxDestroyFont(gameOverFont);
        gameOverFont = -1;
    }

    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    Graphics::Shutdown();
    level.Shutdown();
    DestroyGrid();

    if (m_bgmLoaded)
    {
        AEAudioStopGroup(m_bgmGroup);
        m_bgmLoaded = false;
    }

    buildMergeSystem.Shutdown();
}
#pragma endregion