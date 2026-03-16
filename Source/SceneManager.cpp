#include "SceneManager.h"
#include <cassert>

// Include your scene scripts here:
#include "Scene_TowerTest.h"
#include "Scene_LevelEditor.h"
#include "Scene_ShopTest.h"
#include "Scene_EnemyTest.h"
#include "Scene_LoadLevel.h"
#include "Scene_Prototype.h"
#include "Scene_MainMenu.h"
#include "Scene_Settings.h"
#include "Scene_LevelSelect.h"

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
        case SceneID::MainMenu:     return new Scene_MainMenu();
        case SceneID::Settings:     return new Scene_Settings();
        case SceneID::LevelEditor:  return new Scene_LevelEditor();
	    case SceneID::LoadLevel:    return new Scene_LoadLevel();
        case SceneID::ShopTest:     return new Scene_ShopTest();
        case SceneID::TowerTest:    return new Scene_TowerTest();
        case SceneID::EnemyTest:    return new Scene_Enemy();
        case SceneID::Prototype:    return new Scene_Prototype();
        case SceneID::LevelSelect:  return new Scene_LevelSelect();
        default:                    return nullptr;
    }
}