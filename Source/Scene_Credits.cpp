#include "Scene_Credits.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Credits::Init()
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    m_page = 0;

    m_pages = {
        {
            "CREDITS",
            "",
            "Merge Defenders",
            "Developed by: [Your Team Names]",
            "",
            "Programming:",
            "- Name 1",
            "- Name 2",
            "- Name 3"
        },
        {
            "ASSETS USED",
            "",
            "buggy-font.ttf",
            "bouken.mp3",
            "spritesheet.png",
            "item_window.png",
            "refresh_icon.png",
            "tutorial_01 to tutorial_06",
            "codex.png"
        },
        {
            "SPECIAL THANKS",
            "",
            "DigiPen Institute of Technology",
            "Course instructors",
            "Playtesters",
            "Team members"
        }
    };
}

void Scene_Credits::Update(float)
{
    if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_BACK))
    {
        SceneManager::I().SwitchTo(SceneID::MainMenu);
        return;
    }

    if (AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT) || AEInputCheckTriggered(AEVK_SPACE))
        NextPage();

    if (AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT))
        PrevPage();
}

void Scene_Credits::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);
    if (m_uiFont < 0 || m_page < 0 || m_page >= (int)m_pages.size())
        return;

    float y = 120.0f;
    for (size_t i = 0; i < m_pages[m_page].size(); ++i)
    {
        float scale = (i == 0) ? 1.2f : 0.9f;
        float shade = (i == 0) ? 1.0f : 0.8f;
        AEGfxPrint(m_uiFont, m_pages[m_page][i].c_str(),
            -0.85f, 1.0f - (y / (float)AEGfxGetWindowHeight()) * 2.0f,
            scale, shade, shade, shade, 1.0f);
        y += (i == 0) ? 50.0f : 34.0f;
    }

    char buf[32];
    sprintf_s(buf, "PAGE %d / %d", m_page + 1, (int)m_pages.size());
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.92f, 0.8f, 1, 1, 1, 1);
    AEGfxPrint(m_uiFont, "LEFT/RIGHT or A/D to navigate   ESC to return", -0.95f, 0.84f, 0.8f, 0.8f, 0.8f, 1, 1);
}

void Scene_Credits::Exit()
{
    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }
}

void Scene_Credits::NextPage()
{
    if (m_page < (int)m_pages.size() - 1)
        ++m_page;
}

void Scene_Credits::PrevPage()
{
    if (m_page > 0)
        --m_page;
}