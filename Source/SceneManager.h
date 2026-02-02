#pragma once
#include "SceneID.h"
#include "Scene.h"

class SceneManager
{
public:
    static SceneManager& I();

    void Init(SceneID startScene);
    void Update(float dt);
    void Draw();
    void Exit();

    void SwitchTo(SceneID next);

    SceneID Current() const { return currentId; }

private:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    Scene* CreateScene(SceneID id); // centralized registration

private:
    Scene* currentScene = nullptr;
    SceneID currentId = SceneID::None;
};