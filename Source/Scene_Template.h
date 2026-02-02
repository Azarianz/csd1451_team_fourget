#pragma once
#include "Scene.h"

class Scene_Example : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;
};