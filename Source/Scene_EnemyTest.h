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
    std::vector<Point> myPath; // The path points
};