#include "Scene_Template.h"
#include "AEEngine.h"

void Scene_Template::Init()
{
    // one-time setup
}

void Scene_Template::Update(float /*dt*/)
{
    // logic
}

void Scene_Template::Draw()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);
}

void Scene_Template::Exit()
{
    // cleanup (optional)
}