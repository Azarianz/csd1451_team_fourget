#pragma once
#include "Scene.h"
#include "SceneID.h"
#include "LevelEditor.h"

class Scene_LevelEditor : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

    static void SetReturnScene(SceneID id);

private:
    static SceneID s_returnScene;

    LevelEditor editor;
    bool initialized = false;
};