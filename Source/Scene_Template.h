#pragma once
#include "Scene.h"

class Scene_Template : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;
};