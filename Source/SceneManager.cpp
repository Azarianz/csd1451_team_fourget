#include "SceneManager.h"
#include <cassert>

// Include your scene scripts here:
#include "Scene_TowerTest.h"
#include "Scene_LevelEditor.h"
#include "Scene_Level1.h"
#include "Scene_ShopTest.h"
#include "Scene_EnemyTest.h"
#include "Scene_LoadLevel.h"

SceneManager& SceneManager::I()
{
    static SceneManager inst;
    return inst;
}

void SceneManager::Init(SceneID startScene)
{
    SwitchTo(startScene);
}

void SceneManager::Update(float dt)
{
    if (currentScene)
        currentScene->Update(dt);
}

void SceneManager::Draw()
{
    if (currentScene)
        currentScene->Draw();
}

void SceneManager::Exit()
{
    if (currentScene)
    {
        currentScene->Exit();
        delete currentScene;
        currentScene = nullptr;
    }
    currentId = SceneID::None;
}

void SceneManager::SwitchTo(SceneID next)
{
    // Exit old
    if (currentScene)
    {
        currentScene->Exit();
        delete currentScene;
        currentScene = nullptr;
    }

    // Create new
    currentScene = CreateScene(next);
    currentId = next;

    assert(currentScene && "CreateScene() returned nullptr. Did you register the scene?");
    currentScene->Init();
}

Scene* SceneManager::CreateScene(SceneID id)
{
    switch (id)
    {
        case SceneID::LevelEditor:  return new Scene_LevelEditor();
	    case SceneID::LoadLevel:    return new Scene_LoadLevel();
        case SceneID::Level1:       return new Scene_Level1();
        case SceneID::ShopTest:    return new Scene_ShopTest();
        case SceneID::TowerTest:    return new Scene_TowerTest();
        case SceneID::EnemyTest:    return new Scene_Enemy();
        default:                    return nullptr;
    }
}