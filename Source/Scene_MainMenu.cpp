#include "Scene_MainMenu.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include "GameSettings.h"

#include <cstring>

namespace
{
    constexpr float kBgR = 0.08f;
    constexpr float kBgG = 0.08f;
    constexpr float kBgB = 0.12f;

    constexpr float kBaseTextScale = 1.0f;
    constexpr float kTitleScale = 1.4f;
    constexpr float kControlsScale = 0.9f;
    constexpr float kControlsLineScale = 0.8f;

    constexpr float kApproxCharWidth = 22.0f;

    constexpr float kTitleYRatio = 0.24f;
    constexpr float kControlsYRatio = 0.72f;

    // Fixed menu layout
    constexpr float kButtonStartY = 380.0f;
    constexpr float kButtonSpacing = 62.0f;
    constexpr float kButtonHeight = 40.0f;
    constexpr float kButtonPadX = 18.0f;

    // IMPORTANT:
    // AEGfxPrint Y behaves more like a baseline than a top-left corner.
    // So to make text appear visually centered inside a 40px box,
    // we place the text baseline lower than the box midpoint.
    constexpr float kTextBaselineInsetY = 30.0f;

    constexpr float kPanelBorderThickness = 2.0f;
    constexpr float kArrowOffsetX = 28.0f;

    constexpr float kBright = 1.0f;
    constexpr float kDim = 0.35f;
    constexpr float kInfo = 0.7f;
}

#pragma region Main

void Scene_MainMenu::Init()
{
    selectedOption = MenuOption::Play;

    if (m_uiFont < 0)
    {
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    }
}

void Scene_MainMenu::Update(float /*dt*/)
{
    UpdateMouseInput();

    if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_UP))
    {
        MoveUp();
    }

    if (AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_DOWN))
    {
        MoveDown();
    }

    if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
    {
        ConfirmSelection();
    }
}

void Scene_MainMenu::Draw()
{
    AEGfxSetBackgroundColor(kBgR, kBgG, kBgB);
    DrawUI();
}

void Scene_MainMenu::Exit()
{
    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }
}

#pragma endregion

#pragma region InputAndSelection

void Scene_MainMenu::UpdateMouseInput()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const ButtonRect playRect = GetPlayButtonRect();
    const ButtonRect settingsRect = GetSettingsButtonRect();
    const ButtonRect quitRect = GetQuitButtonRect();

    if (IsPointInRect((float)mouseX, (float)mouseY, playRect.x, playRect.y, playRect.w, playRect.h))
    {
        selectedOption = MenuOption::Play;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            HandlePlay();
        }
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, settingsRect.x, settingsRect.y, settingsRect.w, settingsRect.h))
    {
        selectedOption = MenuOption::Settings;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            HandleSettings();
        }
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, quitRect.x, quitRect.y, quitRect.w, quitRect.h))
    {
        selectedOption = MenuOption::Quit;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            HandleQuit();
        }
    }
}

void Scene_MainMenu::MoveUp()
{
    switch (selectedOption)
    {
    case MenuOption::Play:
        selectedOption = MenuOption::Quit;
        break;

    case MenuOption::Settings:
        selectedOption = MenuOption::Play;
        break;

    case MenuOption::Quit:
        selectedOption = MenuOption::Settings;
        break;
    }
}

void Scene_MainMenu::MoveDown()
{
    switch (selectedOption)
    {
    case MenuOption::Play:
        selectedOption = MenuOption::Settings;
        break;

    case MenuOption::Settings:
        selectedOption = MenuOption::Quit;
        break;

    case MenuOption::Quit:
        selectedOption = MenuOption::Play;
        break;
    }
}

void Scene_MainMenu::ConfirmSelection()
{
    switch (selectedOption)
    {
    case MenuOption::Play:
        HandlePlay();
        break;

    case MenuOption::Settings:
        HandleSettings();
        break;

    case MenuOption::Quit:
        HandleQuit();
        break;

    default:
        break;
    }
}

void Scene_MainMenu::HandlePlay()
{
    SceneManager::I().SwitchTo(SceneID::LevelSelect);
}

void Scene_MainMenu::HandleSettings()
{
    SceneManager::I().SwitchTo(SceneID::Settings);
}

void Scene_MainMenu::HandleQuit()
{
    GameSettings::quitGame = true;
}

#pragma endregion

#pragma region LayoutHelpers

float Scene_MainMenu::ScreenToNormX(float px) const
{
    return (px / AEGfxGetWindowWidth()) * 2.0f - 1.0f;
}

float Scene_MainMenu::ScreenToNormY(float py) const
{
    return 1.0f - (py / AEGfxGetWindowHeight()) * 2.0f;
}

float Scene_MainMenu::GetCenteredX(const char* text, float scale) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float centerX = screenW * 0.5f;
    const float textWidth = (float)std::strlen(text) * kApproxCharWidth * scale;

    return centerX - textWidth * 0.5f;
}

bool Scene_MainMenu::IsPointInRect(float mx, float my, float x, float y, float w, float h) const
{
    return (mx >= x && mx <= x + w &&
        my >= y && my <= y + h);
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetPlayButtonRect() const
{
    const char* text = "PLAY";
    const float textW = (float)std::strlen(text) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY;
    return rect;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetSettingsButtonRect() const
{
    const char* text = "SETTINGS";
    const float textW = (float)std::strlen(text) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY + kButtonSpacing;
    return rect;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetQuitButtonRect() const
{
    const char* text = "QUIT";
    const float textW = (float)std::strlen(text) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY + (kButtonSpacing * 2.0f);
    return rect;
}

#pragma endregion

#pragma region DrawingHelpers

void Scene_MainMenu::DrawPanelPx(float x, float y, float w, float h,
    float r, float g, float b, float a) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(a);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    const unsigned int fillColor =
        (0xFFu << 24) |
        ((unsigned int)(r * 255.0f) << 16) |
        ((unsigned int)(g * 255.0f) << 8) |
        ((unsigned int)(b * 255.0f));

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, fillColor, 0.0f, 0.0f,
        0.5f, -0.5f, fillColor, 0.0f, 0.0f,
        0.5f, 0.5f, fillColor, 0.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, fillColor, 0.0f, 0.0f,
        0.5f, 0.5f, fillColor, 0.0f, 0.0f,
        -0.5f, 0.5f, fillColor, 0.0f, 0.0f);
    AEGfxVertexList* panelMesh = AEGfxMeshEnd();

    if (panelMesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        const float centerX = x + (w * 0.5f);
        const float centerY = y + (h * 0.5f);

        AEMtx33Scale(&scaleM, w, h);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, centerX - screenW * 0.5f, screenH * 0.5f - centerY);

        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);

        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(panelMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(panelMesh);
    }

    const unsigned int borderColor = 0xFFFFFFFF;

    auto DrawBorderPiece = [&](float bx, float by, float bw, float bh)
        {
            AEGfxMeshStart();
            AEGfxTriAdd(-0.5f, -0.5f, borderColor, 0.0f, 0.0f,
                0.5f, -0.5f, borderColor, 0.0f, 0.0f,
                0.5f, 0.5f, borderColor, 0.0f, 0.0f);
            AEGfxTriAdd(-0.5f, -0.5f, borderColor, 0.0f, 0.0f,
                0.5f, 0.5f, borderColor, 0.0f, 0.0f,
                -0.5f, 0.5f, borderColor, 0.0f, 0.0f);
            AEGfxVertexList* mesh = AEGfxMeshEnd();

            if (mesh)
            {
                AEMtx33 scaleM, rotM, transM, finalMtx;
                const float centerX = bx + (bw * 0.5f);
                const float centerY = by + (bh * 0.5f);

                AEMtx33Scale(&scaleM, bw, bh);
                AEMtx33Rot(&rotM, 0.0f);
                AEMtx33Trans(&transM, centerX - screenW * 0.5f, screenH * 0.5f - centerY);

                AEMtx33Concat(&finalMtx, &rotM, &scaleM);
                AEMtx33Concat(&finalMtx, &transM, &finalMtx);

                AEGfxSetTransform(finalMtx.m);
                AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
                AEGfxMeshFree(mesh);
            }
        };

    DrawBorderPiece(x, y, w, kPanelBorderThickness);
    DrawBorderPiece(x, y + h - kPanelBorderThickness, w, kPanelBorderThickness);
    DrawBorderPiece(x, y, kPanelBorderThickness, h);
    DrawBorderPiece(x + w - kPanelBorderThickness, y, kPanelBorderThickness, h);

    AEGfxSetTransparency(1.0f);
}

#pragma endregion

#pragma region UIDrawing

void Scene_MainMenu::DrawUI() const
{
    if (m_uiFont < 0)
        return;

    const float screenH = (float)AEGfxGetWindowHeight();
    const float titleY = screenH * kTitleYRatio;
    const float controlsY = screenH * kControlsYRatio;

    auto Print = [&](const char* text, float px, float py, float shade, float scale = 1.0f)
        {
            AEGfxPrint(
                m_uiFont,
                text,
                ScreenToNormX(px),
                ScreenToNormY(py),
                scale,
                shade, shade, shade, 1.0f
            );
        };

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const ButtonRect playRect = GetPlayButtonRect();
    const ButtonRect settingsRect = GetSettingsButtonRect();
    const ButtonRect quitRect = GetQuitButtonRect();

    const bool playHover =
        IsPointInRect((float)mouseX, (float)mouseY, playRect.x, playRect.y, playRect.w, playRect.h);
    const bool settingsHover =
        IsPointInRect((float)mouseX, (float)mouseY, settingsRect.x, settingsRect.y, settingsRect.w, settingsRect.h);
    const bool quitHover =
        IsPointInRect((float)mouseX, (float)mouseY, quitRect.x, quitRect.y, quitRect.w, quitRect.h);

    Print("MERGE DEFENDERS", GetCenteredX("MERGE DEFENDERS", kTitleScale), titleY, kBright, kTitleScale);

    DrawPanelPx(playRect.x, playRect.y, playRect.w, playRect.h,
        playHover ? 0.35f : 0.12f,
        playHover ? 0.35f : 0.12f,
        playHover ? 0.42f : 0.18f,
        1.0f);

    DrawPanelPx(settingsRect.x, settingsRect.y, settingsRect.w, settingsRect.h,
        settingsHover ? 0.35f : 0.12f,
        settingsHover ? 0.35f : 0.12f,
        settingsHover ? 0.42f : 0.18f,
        1.0f);

    DrawPanelPx(quitRect.x, quitRect.y, quitRect.w, quitRect.h,
        quitHover ? 0.35f : 0.12f,
        quitHover ? 0.35f : 0.12f,
        quitHover ? 0.42f : 0.18f,
        1.0f);

    const char* playText = "PLAY";
    const char* settingsText = "SETTINGS";
    const char* quitText = "QUIT";

    const float playTextX = GetCenteredX(playText, kBaseTextScale);
    const float settingsTextX = GetCenteredX(settingsText, kBaseTextScale);
    const float quitTextX = GetCenteredX(quitText, kBaseTextScale);

    // Baseline positions tied directly to each box.
    const float playTextY = playRect.y + kTextBaselineInsetY;
    const float settingsTextY = settingsRect.y + kTextBaselineInsetY;
    const float quitTextY = quitRect.y + kTextBaselineInsetY;

    Print(playText, playTextX, playTextY,
        (selectedOption == MenuOption::Play || playHover) ? kBright : kDim, kBaseTextScale);

    Print(settingsText, settingsTextX, settingsTextY,
        (selectedOption == MenuOption::Settings || settingsHover) ? kBright : kDim, kBaseTextScale);

    Print(quitText, quitTextX, quitTextY,
        (selectedOption == MenuOption::Quit || quitHover) ? kBright : kDim, kBaseTextScale);

    if (selectedOption == MenuOption::Play)
    {
        Print(">", playRect.x - kArrowOffsetX, playTextY, kBright, kBaseTextScale);
    }
    else if (selectedOption == MenuOption::Settings)
    {
        Print(">", settingsRect.x - kArrowOffsetX, settingsTextY, kBright, kBaseTextScale);
    }
    else
    {
        Print(">", quitRect.x - kArrowOffsetX, quitTextY, kBright, kBaseTextScale);
    }

    Print("CONTROLS:", GetCenteredX("CONTROLS:", kControlsScale), controlsY, kInfo, kControlsScale);

    float y = controlsY + 35.0f;
    Print("W / UP    - Move", GetCenteredX("W / UP    - Move", kControlsLineScale), y, kInfo, kControlsLineScale);

    y += 28.0f;
    Print("S / DOWN  - Move", GetCenteredX("S / DOWN  - Move", kControlsLineScale), y, kInfo, kControlsLineScale);

    y += 28.0f;
    Print("ENTER     - Select", GetCenteredX("ENTER     - Select", kControlsLineScale), y, kInfo, kControlsLineScale);
}

#pragma endregion