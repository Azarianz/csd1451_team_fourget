#include "TutorialPopup.h"
#include "AEInput.h"
#include "AEMath.h"

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

bool TutorialPopup::IsPointInRect(float mx, float my, float x, float y, float w, float h) const
{
    return (mx >= x && mx <= x + w &&
        my >= y && my <= y + h);
}

TutorialPopup::ButtonRect TutorialPopup::GetLeftArrowRect() const
{
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect rect;
    rect.w = 80.0f;
    rect.h = 80.0f;
    rect.x = 24.0f;
    rect.y = screenH * 0.5f - rect.h * 0.5f;
    return rect;
}

TutorialPopup::ButtonRect TutorialPopup::GetRightArrowRect() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect rect;
    rect.w = 80.0f;
    rect.h = 80.0f;
    rect.x = screenW - rect.w - 24.0f;
    rect.y = screenH * 0.5f - rect.h * 0.5f;
    return rect;
}

void TutorialPopup::UpdateMouseInput()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const ButtonRect leftRect = GetLeftArrowRect();
    const ButtonRect rightRect = GetRightArrowRect();

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsPointInRect((float)mouseX, (float)mouseY, leftRect.x, leftRect.y, leftRect.w, leftRect.h))
        {
            if (m_currentSlide > 0)
                --m_currentSlide;
        }
        else if (IsPointInRect((float)mouseX, (float)mouseY, rightRect.x, rightRect.y, rightRect.w, rightRect.h))
        {
            if (m_currentSlide < (int)m_slideTextures.size() - 1)
                ++m_currentSlide;
            else
                Close();
        }
    }
}

void TutorialPopup::Update()
{
    if (!m_active)
        return;

    UpdateMouseInput();

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

void TutorialPopup::DrawNavButtons(int fontId) const
{
    if (fontId < 0)
        return;

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const ButtonRect leftRect = GetLeftArrowRect();
    const ButtonRect rightRect = GetRightArrowRect();

    const bool leftHover =
        IsPointInRect((float)mouseX, (float)mouseY, leftRect.x, leftRect.y, leftRect.w, leftRect.h);
    const bool rightHover =
        IsPointInRect((float)mouseX, (float)mouseY, rightRect.x, rightRect.y, rightRect.w, rightRect.h);

    const bool canGoLeft = (m_currentSlide > 0);
    const bool canGoRight = (m_currentSlide < (int)m_slideTextures.size() - 1);

    auto PrintPx = [&](const char* text, float px, float py, float shade, float scale)
        {
            const float nx = (px / (float)AEGfxGetWindowWidth()) * 2.0f - 1.0f;
            const float ny = 1.0f - (py / (float)AEGfxGetWindowHeight()) * 2.0f;

            AEGfxPrint((s8)fontId, text, nx, ny, scale, shade, shade, shade, 1.0f);
        };

    PrintPx("<",
        leftRect.x + 16.0f,
        leftRect.y + 52.0f,
        canGoLeft ? (leftHover ? 1.0f : 0.55f) : 0.20f,
        1.8f);

    PrintPx(">",
        rightRect.x + 16.0f,
        rightRect.y + 52.0f,
        canGoRight ? (rightHover ? 1.0f : 0.55f) : 0.20f,
        1.8f);
}

void TutorialPopup::Draw(int fontId) const
{
    if (!m_active || m_slideTextures.empty() || !m_imageQuad)
        return;

    const float screenW = static_cast<float>(AEGfxGetWindowWidth());
    const float screenH = static_cast<float>(AEGfxGetWindowHeight());

    AEGfxTexture* tex = m_slideTextures[(size_t)m_currentSlide];
    if (tex)
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(tex, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        AEMtx33 scale, rot, trans, finalMtx;
        AEMtx33Scale(&scale, screenW, screenH);
        AEMtx33Rot(&rot, 0.0f);
        AEMtx33Trans(&trans, 0.0f, 0.0f);

        AEMtx33Concat(&finalMtx, &rot, &scale);
        AEMtx33Concat(&finalMtx, &trans, &finalMtx);

        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
    }

    DrawNavButtons(fontId);

    if (fontId >= 0)
    {
        char pageBuf[64];
        sprintf_s(pageBuf, "Tutorial %d / %d",
            m_currentSlide + 1,
            (int)m_slideTextures.size());

        const char* help = "LEFT/RIGHT or A/D to navigate   ESC = CLOSE";

        float pageWidth = 0.0f, pageHeight = 0.0f;
        AEGfxGetPrintSize((s8)fontId, pageBuf, 1.0f, &pageWidth, &pageHeight);

        float helpWidth = 0.0f, helpHeight = 0.0f;
        AEGfxGetPrintSize((s8)fontId, help, 0.85f, &helpWidth, &helpHeight);

        const float pageX = -(pageWidth * 0.5f);
        const float helpX = -(helpWidth * 0.5f);

        AEGfxPrint((s8)fontId, pageBuf, pageX, 0.85f, 1.0f, 1, 1, 1, 1);
        AEGfxPrint((s8)fontId, help, helpX, 0.8f, 0.85f, 1, 1, 1, 1);
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