// Scene_Prototype.h
#pragma once
#include "Scene.h"
#include "GridSystem.h"
#include "LevelLoader.h"
#include "Enemy.h"
#include "Shop.h"
#include "Tower.h"
#include "GameSettings.h"
#include "BuildMergeSystem.h"
#include "TutorialPopup.h"
#include "ParticleSystem.h"

#include <vector>
#include <cstdint>
#include <string>

class Scene_Prototype : public Scene
{
public:
    void SetLevelFile(const std::string& file) { levelFile = file; }

    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    void DestroyGrid();

    bool InitLevelAndGrid();
    void InitAudio();
    void ResetRuntimeState();
    void CreateBaseTower();

    void HandleUserInputs(float worldX, float worldY, int mouseX, int mouseY);
    void UpdateSelectionAndDragging(float worldX, float worldY, int mouseX, int mouseY);
    void UpdateEnemies(float dt);
    void UpdateBaseCollision();
    void CleanupDeadEnemies();
    void UpdateReturningTowers(float dt);

    bool IsStageCleared() const;
    bool LoadNextLevel();


	// Win Popup
    void OpenWinPopup();
    void UpdateWinPopup(int mouseX, int mouseY);
    void DrawWinPopup() const;
    bool IsInNextStageButton(int mouseX, int mouseY) const;
    bool IsInMainMenuButton(int mouseX, int mouseY) const;

private:
    TutorialPopup m_tutorialPopup;
    bool IsTutorialLevel() const;
    void UpdateTutorialPopup();

    std::string levelFile;
    LevelLoader level;
    GridSystem::Grid* grid = nullptr;

    int baseTowerIndex = -1;
    bool gameOver = false;
    bool m_stageWon = false;
    s8 gameOverFont = -1;

    WaveManager waveManager;
    s8 m_uiFont = -1;
    void DrawUI();

    std::vector<Point> path;
    std::vector<Enemy*> enemies;

    TowerHandler::Shop shop;
    BuildMergeSystem buildMergeSystem;
    std::vector<TowerHandler::Tower> activeTowers;
    std::vector<TowerHandler::ActiveBullet> activeBullets;

    AEAudio      m_bgm = AEAudio();
    AEAudioGroup m_bgmGroup = AEAudioGroup();
    bool         m_bgmLoaded = false;

    std::vector<uint8_t> occupied;

    bool wasLmbDown = false;
    bool m_paused = false;
    bool IsPauseButtonClicked(int mouseX, int mouseY) const;

    float gameSpeedMultiplier = 1.0f;
    AEGfxTexture* m_spriteSheet = nullptr;
    AEGfxVertexList* m_flagMeshes[3] = { nullptr };
};