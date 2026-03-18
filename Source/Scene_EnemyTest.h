#pragma once
#include "Scene.h"
#include "Enemy.h"
#include <vector>

class Scene_Enemy : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    std::vector<Enemy*> activeEnemies;
    std::vector<Point> myPath;

    WaveManager waveManager;
    s8 m_uiFont = -1;
    void DrawUI();

    // Add game speed multiplier
    float gameSpeedMultiplier = 1.0f;

    AEGfxTexture* m_spriteSheet = nullptr;
    AEGfxVertexList* m_flagMeshes[3] = { nullptr };
};