#pragma once
#include "Scene.h"
#include "LevelEditor.h"

class Scene_LevelEditor : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    LevelEditor editor;
    bool initialized = false;
};