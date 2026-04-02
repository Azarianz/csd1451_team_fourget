#include "Scene_Credits.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include "AEMath.h"

void Scene_Credits::Init()
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    m_page = 0;

    // Load first credits page as an image
    m_firstPageTex = AEGfxTextureLoad("Assets/copyright.png");
    if (m_firstPageTex)
    {
        AEGfxMeshStart();
        AEGfxTriAdd(
            -0.9f, -0.7f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.9f, -0.7f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.9f, 0.7f, 0xFFFFFFFF, 1.0f, 0.0f);

        AEGfxTriAdd(
            -0.9f, -0.7f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.9f, 0.7f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.9f, 0.7f, 0xFFFFFFFF, 0.0f, 0.0f);

        m_firstPageMesh = AEGfxMeshEnd();
    }

    // Text pages start from page 1 onward
    m_pages = {
        {
            "---- Faculties & Advisors ----",
            "PRESIDENT",
            "Claude Comair",
            "",
            "Acting Department Chair",
            "Prasanna Kumar Ghali",
            "",
            "Instructors",
            "Wong Han Feng Gerald",
            "Dr. Soroor Malek Mohammadai Faradounbeh",
            "Tan Chee Wei Tommy"
        },
        {
            "CREDITS",
            "",
            "Merge Defenders",
            "Developed by: Team Fourget",
            "",
            "Programming:",
            "Azarian",
            "Tze Siong",
            "Adley",
            "Caleb"
        },
        {
            "ASSETS USED",
            "",
            "buggy-font.ttf",
            "bouken.mp3 - MAKOOTO",
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

    if (m_page == 0)
        DrawImagePage();
    else
        DrawTextPage();

    char buf[32];
    sprintf_s(buf, "PAGE %d / %d", m_page + 1, (int)m_pages.size() + 1);
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.92f, 0.8f, 1, 1, 1, 1);
    AEGfxPrint(m_uiFont, "LEFT/RIGHT or A/D to navigate   ESC to return", -0.95f, 0.84f, 0.8f, 0.8f, 0.8f, 1, 1);
}

void Scene_Credits::DrawImagePage()
{
    if (!m_firstPageTex || !m_firstPageMesh)
        return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_firstPageTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    AEMtx33 trans;
    AEMtx33Identity(&trans);
    AEGfxSetTransform(trans.m);

    AEGfxMeshDraw(m_firstPageMesh, AE_GFX_MDM_TRIANGLES);
}

void Scene_Credits::DrawTextPage()
{
    if (m_uiFont < 0)
        return;

    int textIndex = m_page - 1;
    if (textIndex < 0 || textIndex >= (int)m_pages.size())
        return;

    float y = 120.0f;
    for (size_t i = 0; i < m_pages[textIndex].size(); ++i)
    {
        float scale = (i == 0) ? 1.2f : 0.9f;
        float shade = (i == 0) ? 1.0f : 0.8f;

        AEGfxPrint(
            m_uiFont,
            m_pages[textIndex][i].c_str(),
            -0.85f,
            1.0f - (y / (float)AEGfxGetWindowHeight()) * 2.0f,
            scale,
            shade, shade, shade, 1.0f
        );

        y += (i == 0) ? 50.0f : 34.0f;
    }
}

void Scene_Credits::Exit()
{
    if (m_firstPageMesh)
    {
        AEGfxMeshFree(m_firstPageMesh);
        m_firstPageMesh = nullptr;
    }

    if (m_firstPageTex)
    {
        AEGfxTextureUnload(m_firstPageTex);
        m_firstPageTex = nullptr;
    }

    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }
}

void Scene_Credits::NextPage()
{
    int totalPages = (int)m_pages.size() + 1;
    if (m_page < totalPages - 1)
        ++m_page;
}

void Scene_Credits::PrevPage()
{
    if (m_page > 0)
        --m_page;
}