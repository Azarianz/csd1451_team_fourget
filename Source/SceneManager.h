#pragma once
#include "SceneID.h"
#include "Scene.h"
#include "AEEngine.h"

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
    void SetBGMVolume(float volume);

private:
    SceneManager() = default;
    ~SceneManager() = default;

    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    Scene* CreateScene(SceneID id); // centralized registration

private:
    Scene* currentScene = nullptr;
    SceneID currentId = SceneID::None;

    AEAudio      m_persistentBgm;
    AEAudioGroup m_bgmGroup;
    bool         m_audioInitialized = false;
};