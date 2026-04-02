#include "Scene_Controls.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"

void Scene_Controls::Init()
{
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    m_currentSlide = 0;

    m_slidePaths = {
        "Assets/Tutorial/tutorial_01.png",
        "Assets/Tutorial/tutorial_02.png",
        "Assets/Tutorial/tutorial_03.png",
        "Assets/Tutorial/tutorial_04.png",
        "Assets/Tutorial/tutorial_05.png",
        "Assets/Tutorial/tutorial_06.png",
        "Assets/codex.png"
    };

    m_slides.clear();
    for (const std::string& path : m_slidePaths)
        m_slides.push_back(AEGfxTextureLoad(path.c_str()));
}

void Scene_Controls::Update(float)
{
    if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_BACK))
    {
        SceneManager::I().SwitchTo(SceneID::MainMenu);
        return;
    }

    if (AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT) || AEInputCheckTriggered(AEVK_SPACE))
        NextSlide();

    if (AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT))
        PrevSlide();
}

void Scene_Controls::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);

    if (m_currentSlide >= 0 && m_currentSlide < (int)m_slides.size() && m_slides[m_currentSlide])
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(m_slides[m_currentSlide], 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1, 1, 1, 1);

        AEGfxMeshStart();
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 1, 0.5f, -0.5f, 0xFFFFFFFF, 1, 1, 0.5f, 0.5f, 0xFFFFFFFF, 1, 0);
        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 1, 0.5f, 0.5f, 0xFFFFFFFF, 1, 0, -0.5f, 0.5f, 0xFFFFFFFF, 0, 0);
        AEGfxVertexList* mesh = AEGfxMeshEnd();

        if (mesh)
        {
            AEMtx33 scale, trans, finalMtx;
            AEMtx33Scale(&scale, 1100.0f, 700.0f);
            AEMtx33Trans(&trans, 0.0f, 0.0f);
            AEMtx33Concat(&finalMtx, &trans, &scale);
            AEGfxSetTransform(finalMtx.m);
            AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
            AEGfxMeshFree(mesh);
        }
    }

    if (m_uiFont >= 0)
    {
        char buf[64];
        sprintf_s(buf, "CONTROLS  %d / %d", m_currentSlide + 1, (int)m_slides.size());
        AEGfxPrint(m_uiFont, buf, -0.95f, 0.92f, 1.0f, 1, 1, 1, 1);
        AEGfxPrint(m_uiFont, "LEFT/RIGHT or A/D to navigate   ESC to return",
            -0.95f, 0.84f, 0.8f, 0.8f, 0.8f, 0.8f, 1.0f);
    }
}

void Scene_Controls::Exit()
{
    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    for (AEGfxTexture*& tex : m_slides)
    {
        if (tex)
        {
            AEGfxTextureUnload(tex);
            tex = nullptr;
        }
    }
    m_slides.clear();
}

void Scene_Controls::NextSlide()
{
    if (m_currentSlide < (int)m_slides.size() - 1)
        ++m_currentSlide;
}

void Scene_Controls::PrevSlide()
{
    if (m_currentSlide > 0)
        --m_currentSlide;
}