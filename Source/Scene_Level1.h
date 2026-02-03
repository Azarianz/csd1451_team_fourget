#pragma once
#include "Scene.h"
#include "GridSystem.h"
#include "LevelLoader.h"

class Scene_Level1 : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    LevelLoader level;
    GridSystem::Grid* grid = nullptr;

    const char* levelPath = "Assets/Levels/Level_01.txt";
};