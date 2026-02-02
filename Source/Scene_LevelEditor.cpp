#include "Scene_LevelEditor.h"

#include "AEEngine.h"
#include "AEInput.h"

void Scene_LevelEditor::Init()
{
    editor.Init(20, 12); // grid size
    initialized = true;
}

void Scene_LevelEditor::Update(float dt)
{
    if (!initialized)
        return;

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