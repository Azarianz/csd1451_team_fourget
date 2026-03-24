#include "TutorialPopup.h"
#include "AEInput.h"

namespace
{
    AEGfxVertexList* CreateTexturedQuad()
    {
        AEGfxMeshStart();
        AEGfxTriAdd(
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);

        AEGfxTriAdd(
            -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
        return AEGfxMeshEnd();
    }

    AEGfxVertexList* CreateColorQuad(unsigned int color)
    {
        AEGfxMeshStart();
        AEGfxTriAdd(
            -0.5f, -0.5f, color, 0.0f, 0.0f,
            0.5f, -0.5f, color, 0.0f, 0.0f,
            0.5f, 0.5f, color, 0.0f, 0.0f);

        AEGfxTriAdd(
            -0.5f, -0.5f, color, 0.0f, 0.0f,
            0.5f, 0.5f, color, 0.0f, 0.0f,
            -0.5f, 0.5f, color, 0.0f, 0.0f);
        return AEGfxMeshEnd();
    }
}

void TutorialPopup::Init()
{
    if (!m_imageQuad)
        m_imageQuad = CreateTexturedQuad();
}

void TutorialPopup::Shutdown()
{
    UnloadSlides();

    if (m_imageQuad)
    {
        AEGfxMeshFree(m_imageQuad);
        m_imageQuad = nullptr;
    }
}

void TutorialPopup::SetEnabled(bool enabled)
{
    m_enabled = enabled;
}

void TutorialPopup::SetSlides(const std::vector<std::string>& imagePaths)
{
    m_slidePaths = imagePaths;
    LoadSlides();
}

void TutorialPopup::Start()
{
    if (!m_enabled || m_slideTextures.empty())
        return;

    m_active = true;
    m_startedOnce = true;
    m_finished = false;
    m_currentSlide = 0;
}

void TutorialPopup::ForceStart()
{
    if (m_slideTextures.empty())
        return;

    m_active = true;
    m_startedOnce = true;
    m_finished = false;
    m_currentSlide = 0;
}

void TutorialPopup::Close()
{
    m_active = false;
    m_finished = true;
}

void TutorialPopup::Reset()
{
    m_active = false;
    m_startedOnce = false;
    m_finished = false;
    m_currentSlide = 0;
}

void TutorialPopup::Update()
{
    if (!m_active)
        return;

    if (AEInputCheckTriggered(AEVK_RIGHT) ||
        AEInputCheckTriggered(AEVK_D) ||
        AEInputCheckTriggered(AEVK_SPACE) ||
        AEInputCheckTriggered(AEVK_RETURN))
    {
        if (m_currentSlide < static_cast<int>(m_slideTextures.size()) - 1)
        {
            ++m_currentSlide;
        }
        else
        {
            Close();
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_LEFT) ||
        AEInputCheckTriggered(AEVK_A) ||
        AEInputCheckTriggered(AEVK_BACK))
    {
        if (m_currentSlide > 0)
            --m_currentSlide;
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        Close();
    }
}

void TutorialPopup::Draw(int fontId) const
{
    if (!m_active || m_slideTextures.empty() || !m_imageQuad)
        return;

    const float screenW = static_cast<float>(AEGfxGetWindowWidth());
    const float screenH = static_cast<float>(AEGfxGetWindowHeight());

    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxTexture* tex = m_slideTextures[(size_t)m_currentSlide];
    if (tex)
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(tex, 0, 0);

        AEMtx33 scale, rot, trans, finalMtx;
        AEMtx33Scale(&scale, screenW, screenH);
        AEMtx33Rot(&rot, 0.0f);
        AEMtx33Trans(&trans, 0.0f, 0.0f);

        AEMtx33Concat(&finalMtx, &rot, &scale);
        AEMtx33Concat(&finalMtx, &trans, &finalMtx);

        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
    }

    if (fontId >= 0)
    {
        char pageBuf[64];
        sprintf_s(pageBuf, "Tutorial %d / %d",
            m_currentSlide + 1,
            (int)m_slideTextures.size());

        auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
        auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

        const float textY1 = screenH * 0.86f;
        const float textY2 = screenH * 0.92f;

        AEGfxPrint(fontId, pageBuf,
            ToNdcX(screenW * 0.43f), ToNdcY(textY1),
            1.0f, 1, 1, 1, 1);

        AEGfxPrint(fontId, "LEFT/A = PREV",
            ToNdcX(screenW * 0.14f), ToNdcY(textY2),
            0.85f, 1, 1, 1, 1);

        AEGfxPrint(fontId, "RIGHT/D/ENTER/SPACE = NEXT",
            ToNdcX(screenW * 0.35f), ToNdcY(textY2),
            0.85f, 1, 1, 1, 1);

        AEGfxPrint(fontId, "ESC = CLOSE",
            ToNdcX(screenW * 0.80f), ToNdcY(textY2),
            0.85f, 1, 1, 1, 1);
    }
}

bool TutorialPopup::IsActive() const
{
    return m_active;
}

bool TutorialPopup::HasStarted() const
{
    return m_startedOnce;
}

bool TutorialPopup::IsFinished() const
{
    return m_finished;
}

void TutorialPopup::LoadSlides()
{
    UnloadSlides();

    for (const std::string& path : m_slidePaths)
    {
        AEGfxTexture* tex = AEGfxTextureLoad(path.c_str());
        m_slideTextures.push_back(tex);
    }
}

void TutorialPopup::UnloadSlides()
{
    for (AEGfxTexture* tex : m_slideTextures)
    {
        if (tex)
            AEGfxTextureUnload(tex);
    }
    m_slideTextures.clear();
}