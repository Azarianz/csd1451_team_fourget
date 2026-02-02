#include "Scene_Level1.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Level1::Init()
{
    if (!level.LoadFromText(levelPath))
    {
        PRINT("FAILED to load level text: %s\n", levelPath);
        return;
    }

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();
}

void Scene_Level1::Update(float /*dt*/)
{
    // Optional: hot reload while testing
    if (AEInputCheckTriggered(AEVK_F5))
    {
        level.Shutdown();
        level.LoadFromText(levelPath);

        if (grid)
        {
            grid->Destroy();
            delete grid;
        }
        grid = new GridSystem::Grid(level.width, level.height, 1.0f);
        grid->Init();
    }
}

void Scene_Level1::Draw()
{
    if (!grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    grid->Draw();          // optional
    level.Draw(*grid, 1.0f);
}

void Scene_Level1::Exit()
{
    level.Shutdown();

    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
}