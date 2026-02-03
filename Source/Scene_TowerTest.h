#pragma once
#include "Scene.h"
#include "Tower.h"

class Scene_TowerTest : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    bool initialized = false;
    int mouseX{}, mouseY{};
    TowerHandler::ShopTower shopTower;
    std::vector<TowerHandler::Tower>& activeTowers;
};