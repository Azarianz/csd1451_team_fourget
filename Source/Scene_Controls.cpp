#include "Scene_Controls.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Controls::Init()
{
    if (m_uiFont < 0)
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    m_tutorialPopup.Init();
    m_tutorialPopup.Reset();
    m_tutorialPopup.SetEnabled(true);
    m_tutorialPopup.SetSlides({
        "Assets/Tutorial/tutorial_01.png",
        "Assets/Tutorial/tutorial_02.png",
        "Assets/Tutorial/tutorial_03.png",
        "Assets/Tutorial/tutorial_04.png",
        "Assets/Tutorial/tutorial_05.png",
        "Assets/Tutorial/tutorial_06.png",
        "Assets/codex.png"
        });
    m_tutorialPopup.ForceStart();
}

void Scene_Controls::Update(float)
{
    m_tutorialPopup.Update();

    if (!m_tutorialPopup.IsActive())
    {
        SceneManager::I().SwitchTo(SceneID::MainMenu);
        return;
    }
}

void Scene_Controls::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);
    m_tutorialPopup.Draw(m_uiFont);
}

void Scene_Controls::Exit()
{
    m_tutorialPopup.Shutdown();

    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }
}