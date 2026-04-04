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

    m_pages = {
        {
            "THANKS FOR PLAYING!",
            "",
            "We appreciate you playing Merge Defenders",
            "",
            "Press A/D to view credits ->"
        },
        {
            "FACULTIES & ADVISORS",
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
            "buggy-font.ttf - CodeMan38",
            "bouken.mp3 - MAKOOTO",
            "Tiny Battle 2D - Kenney Assets",
            "Tiny Tower (Bomb Sprite) - Kenney Assets",
            "item_window.png - Asesprite",
            "refresh_icon.png - Asesprite",
            "tutorial_01 to tutorial_06 - Asesprite",
            "codex.png - Asesprite"
        }
    };
}

bool Scene_Credits::IsPointInRect(float mx, float my, float x, float y, float w, float h) const
{
    return (mx >= x && mx <= x + w &&
        my >= y && my <= y + h);
}

Scene_Credits::ButtonRect Scene_Credits::GetLeftArrowRect() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect rect;
    rect.w = 80.0f;
    rect.h = 80.0f;
    rect.x = screenW * 0.5f - 520.0f;
    rect.y = screenH * 0.5f - rect.h * 0.5f;
    return rect;
}

Scene_Credits::ButtonRect Scene_Credits::GetRightArrowRect() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect rect;
    rect.w = 80.0f;
    rect.h = 80.0f;
    rect.x = screenW * 0.5f + 440.0f;
    rect.y = screenH * 0.5f - rect.h * 0.5f;
    return rect;
}

void Scene_Credits::UpdateMouseInput()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const ButtonRect leftRect = GetLeftArrowRect();
    const ButtonRect rightRect = GetRightArrowRect();

    if (IsPointInRect((float)mouseX, (float)mouseY, leftRect.x, leftRect.y, leftRect.w, leftRect.h))
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            PrevPage();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, rightRect.x, rightRect.y, rightRect.w, rightRect.h))
    {
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            NextPage();
    }
}

void Scene_Credits::Update(float)
{
    UpdateMouseInput();

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

void Scene_Credits::DrawNavButtons() const
{
    if (m_uiFont < 0)
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

    auto PrintPx = [&](const char* text, float px, float py, float shade, float scale)
        {
            const float nx = (px / (float)AEGfxGetWindowWidth()) * 2.0f - 1.0f;
            const float ny = 1.0f - (py / (float)AEGfxGetWindowHeight()) * 2.0f;

            AEGfxPrint((s8)m_uiFont, text, nx, ny, scale, shade, shade, shade, 1.0f);
        };

    if (m_page > 0)
        PrintPx("<", leftRect.x + 22.0f, leftRect.y + 52.0f, leftHover ? 1.0f : 0.55f, 1.8f);

    if (m_page < (int)m_pages.size())
        PrintPx(">", rightRect.x + 22.0f, rightRect.y + 52.0f, rightHover ? 1.0f : 0.55f, 1.8f);
}

void Scene_Credits::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);

    if (m_page == 1)
        DrawImagePage();
    else
        DrawTextPage();

    DrawNavButtons();

    char buf[32];
    sprintf_s(buf, "PAGE %d / %d", m_page + 1, (int)m_pages.size() + 1);

    const char* controls = "LEFT/RIGHT or A/D to navigate   ESC to return";

    float pageWidth = 0.0f, pageHeight = 0.0f;
    AEGfxGetPrintSize((s8)m_uiFont, buf, 0.8f, &pageWidth, &pageHeight);

    float ctrlWidth = 0.0f, ctrlHeight = 0.0f;
    AEGfxGetPrintSize((s8)m_uiFont, controls, 0.8f, &ctrlWidth, &ctrlHeight);

    float pageX = -(pageWidth / 2.0f);
    float ctrlX = -(ctrlWidth / 2.0f);

    AEGfxPrint((s8)m_uiFont, buf, pageX, 0.92f, 0.8f, 1, 1, 1, 1);
    AEGfxPrint((s8)m_uiFont, controls, ctrlX, 0.84f, 0.8f, 0.8f, 0.8f, 1, 1);
}

void Scene_Credits::DrawImagePage()
{
    if (!m_firstPageTex)
        return;

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_firstPageTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1, 1, 1, 1);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 1,
        0.5f, -0.5f, 0xFFFFFFFF, 1, 1,
        0.5f, 0.5f, 0xFFFFFFFF, 1, 0);

    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0, 1,
        0.5f, 0.5f, 0xFFFFFFFF, 1, 0,
        -0.5f, 0.5f, 0xFFFFFFFF, 0, 0);

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

void Scene_Credits::DrawTextPage()
{
    if (m_uiFont < 0)
        return;

    int textIndex = 0;

    if (m_page == 0)
        textIndex = 0;          // THANKS FOR PLAYING page
    else if (m_page >= 2)
        textIndex = m_page - 1; // skip over copyright image page at m_page == 1
    else
        return;

    if (textIndex < 0 || textIndex >= (int)m_pages.size())
        return;

    float y = 220.0f;
    float screenH = (float)AEGfxGetWindowHeight();

    for (size_t i = 0; i < m_pages[textIndex].size(); ++i)
    {
        const std::string& text = m_pages[textIndex][i];

        float scale = (i == 0) ? 1.2f : 0.9f;
        float shade = (i == 0) ? 1.0f : 0.8f;

        float width = 0.0f, height = 0.0f;
        AEGfxGetPrintSize((s8)m_uiFont, text.c_str(), scale, &width, &height);

        float x = -(width / 2.0f);
        float yNorm = 1.0f - (y / screenH) * 2.0f;

        AEGfxPrint(
            (s8)m_uiFont,
            text.c_str(),
            x,
            yNorm,
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
        AEGfxDestroyFont((s8)m_uiFont);
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