#include "Scene_LevelEditor.h"

#include "SceneManager.h"
#include "AEEngine.h"
#include "AEInput.h"

SceneID Scene_LevelEditor::s_returnScene = SceneID::MainMenu;

void Scene_LevelEditor::SetReturnScene(SceneID id)
{
    s_returnScene = id;
}

void Scene_LevelEditor::Init()
{
    editor.Init(20, 12); // grid size
    initialized = true;
}

void Scene_LevelEditor::Update(float dt)
{
    if (!initialized)
        return;

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        SceneManager::I().SwitchTo(s_returnScene);
        return;
    }

    editor.Update(dt);
}

void Scene_LevelEditor::Draw()
{
    editor.Draw();
}

void Scene_LevelEditor::Exit()
{
    if (initialized)
    {
        editor.Shutdown();
        initialized = false;
    }
}