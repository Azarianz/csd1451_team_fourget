#include "Scene_Prototype.h"

#include "AEEngine.h"
#include "AEInput.h"
#include "Utility.h"
#include <cstdio>

#pragma region Selection helpers
void Scene_Prototype::ClearTowerSelection()
{
    for (auto& t : activeTowers)
        t.isSelected = false;
}

int  Scene_Prototype::FindPlacedTowerAtMouse(float worldX, float worldY) const
{
    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
    {
        const auto& t = activeTowers[(size_t)i];

        if (t.isDragging)
            continue;

        const float halfW = t._sizeX * 0.5f;
        const float halfH = t._sizeY * 0.5f;

        if (worldX >= t.x - halfW && worldX <= t.x + halfW &&
            worldY >= t.y - halfH && worldY <= t.y + halfH)
        {
            return i;
        }
    }

    return -1;
}

void Scene_Prototype::HandleTowerSelection(float worldX, float worldY, bool justPressedLmb)
{
    if (!justPressedLmb)
        return;

    if (buildMergeSystem.IsDraggingTower())
        return;

    int hitIndex = FindPlacedTowerAtMouse(worldX, worldY);

    ClearTowerSelection();

    if (hitIndex >= 0)
        activeTowers[(size_t)hitIndex].isSelected = true;
}
#pragma endregion

#pragma region Scene lifecycle
void Scene_Prototype::Init()
{
    char levelPath[128] = {};
    sprintf_s(levelPath, "Assets/Levels/level_%02d.txt", levelIndex);

    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
    level.Shutdown();

    if (!level.LoadFromText(levelPath))
    {
        PRINT("FAILED to load level: %s\n", levelPath);
        PRINT("Scene_Prototype Init failed to load level.\n");
        return;
    }

    const size_t expected = (size_t)level.width * (size_t)level.height;
    if (expected == 0 ||
        level.map.size() != expected ||
        level.region.size() != expected)
    {
        PRINT("LEVEL DATA SIZE MISMATCH! w=%d h=%d expected=%zu map=%zu region=%zu\n",
            level.width, level.height,
            expected, level.map.size(), level.region.size());
        return;
    }

    occupied.assign(expected, 0);

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    path.clear();
    if (!level.BuildPath(*grid, path))
    {
        PRINT("FAILED to build enemy path from region flags.\n");
        return;
    }

    TowerHandler::LoadTowerAssets();

    shop.Init();
    buildMergeSystem.Init(&level, grid, &shop, &activeTowers, &activeBullets, &occupied);

    m_bgm = AEAudioLoadMusic("Assets/bouken.mp3");
    m_bgmGroup = AEAudioCreateGroup();
    m_bgmLoaded = true;
    AEAudioPlay(m_bgm, m_bgmGroup, 1.0f, 1.0f, -1);

    float vol = GameSettings::masterVolume / 100.0f;
    AEAudioSetGroupVolume(m_bgmGroup, vol);

    activeTowers.clear();
    activeBullets.clear();
    enemies.clear();

    wasLmbDown = false;
    baseTowerIndex = -1;
    gameOver = false;

    if (!path.empty())
    {
        TowerHandler::ShopTower baseShop;
        baseShop.ShopTowerInit(path.back().x, path.back().y, 80.0f, 80.0f, TowerHandler::BASE_TOWER);

        TowerHandler::Tower baseTower;
        baseTower.TowerInit(path.back().x, path.back().y, 80.0f, 80.0f, baseShop);
        baseTower.isDragging = false;
        baseTower.isSelected = false;

        activeTowers.push_back(baseTower);
        baseTowerIndex = (int)activeTowers.size() - 1;
        buildMergeSystem.RebuildOccupiedFromTowers();
    }

    if (gameOverFont < 0)
        gameOverFont = AEGfxCreateFont("Assets/buggy-font.ttf", 64);

    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    if (!waveManager.LoadFromFile("Assets/waves.txt"))
        PRINT("Failed to load waves.txt!\n");
}

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

void Scene_Prototype::Update(float dt)
{
    if (gameOver) return;
    if (!grid) return;

    int mouseX = 0, mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float worldX = 0.0f, worldY = 0.0f;
    Utility::GetWorldMousePos(worldX, worldY);

    if (AEInputCheckTriggered(AEVK_P))
    {
        m_paused = !m_paused;
        if (m_bgmLoaded)
        {
            if (m_paused) AEAudioPauseGroup(m_bgmGroup);
            else          AEAudioResumeGroup(m_bgmGroup);
        }
    }

    if (AEInputCheckTriggered(AEVK_LBUTTON) && IsPauseButtonClicked(mouseX, mouseY))
    {
        m_paused = !m_paused;
        if (m_bgmLoaded)
        {
            if (m_paused) AEAudioPauseGroup(m_bgmGroup);
            else          AEAudioResumeGroup(m_bgmGroup);
        }
        return;
    }

    if (m_paused) return;

    bool lmbDown = AEInputCheckCurr(AEVK_LBUTTON);
    bool justPressedLmb = (!wasLmbDown && lmbDown);
    bool justReleasedLmb = (wasLmbDown && !lmbDown);

    shop.Update(activeTowers);

    buildMergeSystem.UpdateDragging(worldX, worldY, lmbDown, justReleasedLmb, mouseX, mouseY);

    HandleTowerSelection(worldX, worldY, justPressedLmb);

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

    for (auto& t : activeTowers)
    {
        if (t.details.fireTimer > 0.f)
        {
            t.details.fireTimer -= dt;
            if (t.details.fireTimer < 0.f)
                t.details.fireTimer = 0.f;
        }
    }

    Enemy* spawnedEnemy = waveManager.UpdateAndSpawn(dt, path);
    if (spawnedEnemy)
        enemies.push_back(spawnedEnemy);

    for (Enemy* e : enemies)
    {
        if (e)
            e->Update(dt, path);
    }

    if (baseTowerIndex >= 0 && baseTowerIndex < (int)activeTowers.size())
    {
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

    for (auto& t : activeTowers)
    {
        if (t.isDragging || t.IsBaseTower())
            continue;

        for (Enemy* e : enemies)
        {
            if (!e || e->health <= 0.0f)
                continue;

            if (TowerHandler::TowerShoot(t, *e, activeBullets))
                break;
        }
    }

    TowerHandler::UpdateProjectiles(dt, enemies, activeBullets);

    for (Enemy* e : enemies)
    {
        if (!e || e->health <= 0.0f)
        {
            for (auto& b : activeBullets)
            {
                if (b.target == e)
                    b.target = nullptr;
            }
        }
    }

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

    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
    {
        TowerHandler::Tower& t = activeTowers[(size_t)i];
        if (!t.isReturning) continue;

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

    wasLmbDown = lmbDown;
}

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

    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    Graphics::Shutdown();
    level.Shutdown();

    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }

    if (m_bgmLoaded)
    {
        AEAudioStopGroup(m_bgmGroup);
        m_bgmLoaded = false;
    }

    buildMergeSystem.Shutdown();
}
#pragma endregion
