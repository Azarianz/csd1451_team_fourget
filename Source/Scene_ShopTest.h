#pragma once
#include "Scene.h"
#include "Tower.h"
#include "Shop.h"
#include <vector>

class Scene_ShopTest : public Scene
{
public:

    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    int mouseX{}, mouseY{};
    TowerHandler::Shop shop;
    std::vector<TowerHandler::Tower> activeTowers{};
};