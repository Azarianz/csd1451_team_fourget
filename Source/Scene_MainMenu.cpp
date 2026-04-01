#include "Scene_MainMenu.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include "GameSettings.h"

#include <cstring>
#include <cmath>

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

    constexpr float kTitleYRatio = 0.15f;
    constexpr float kControlsYRatio = 0.68f;

    // Fixed menu layout
    constexpr float kButtonStartY = 200.0f;
    constexpr float kButtonSpacing = 110.0f;
    constexpr float kButtonHeight = 100.0f;
    constexpr float kButtonPadX = 96.0f;

    // FIX: vertically center text inside button — ~58% down accounts for font baseline in a 100px button
    constexpr float kTextBaselineInsetY = kButtonHeight * 0.58f;

    constexpr float kPanelBorderThickness = 2.0f;
    constexpr float kArrowOffsetX = 36.0f;

    constexpr float kBounceSpeed = 7.0f;
    constexpr float kBounceAmplitude = 5.0f;

    constexpr float kBright = 1.0f;
    constexpr float kDim = 0.35f;
    constexpr float kInfo = 0.7f;

    // FIX: increased estimate so copyright right-align math doesn't clip off screen
    constexpr float kFooterCharWidth = 16.0f;
}

#pragma region Main

void Scene_MainMenu::Init()
{
    selectedOption = MenuOption::Play;

    if (m_uiFont < 0)
    {
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
    }

    m_bgm = AEAudioLoadMusic("Assets/bouken.mp3");
    m_bgmGroup = AEAudioCreateGroup();
    m_bgmLoaded = true;

    float normalised = GameSettings::masterVolume / 100.0f;
    AEAudioPlay(m_bgm, m_bgmGroup, 1.0f, 1.0f, -1);
    AEAudioSetGroupVolume(m_bgmGroup, normalised);
}

void Scene_MainMenu::Update(float dt)
{
    m_bounceTimer += dt;

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

    if (m_bgmLoaded)
    {
        AEAudioStopGroup(m_bgmGroup);
        m_bgmLoaded = false;
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
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "SETTINGS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
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
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "SETTINGS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
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
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "SETTINGS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
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

    // Bounce: selected button moves up and down on a sine wave.
    // Subtracting from Y moves the button upward (screen-space Y increases downward).
    const float bounceOffset = sinf(m_bounceTimer * kBounceSpeed) * kBounceAmplitude;

    // Per-button Y offsets: only the selected button bounces.
    const float playOffsetY = (selectedOption == MenuOption::Play) ? -bounceOffset : 0.0f;
    const float settingsOffsetY = (selectedOption == MenuOption::Settings) ? -bounceOffset : 0.0f;
    const float quitOffsetY = (selectedOption == MenuOption::Quit) ? -bounceOffset : 0.0f;

    // Resolve panel colours (C++14-compatible, no tuple/structured bindings)
    struct RGB { float r, g, b; };
    auto PanelColour = [&](MenuOption opt, bool hover) -> RGB
        {
            if (selectedOption == opt)
                return { 0.12f, 0.20f, 0.58f };   // blue-purple highlight
            if (hover)
                return { 0.35f, 0.35f, 0.42f };   // hover tint
            return { 0.12f, 0.12f, 0.18f };        // default dark
        };

    RGB pc = PanelColour(MenuOption::Play, playHover);
    RGB sc = PanelColour(MenuOption::Settings, settingsHover);
    RGB qc = PanelColour(MenuOption::Quit, quitHover);

    DrawPanelPx(playRect.x, playRect.y + playOffsetY, playRect.w, playRect.h, pc.r, pc.g, pc.b, 1.0f);
    DrawPanelPx(settingsRect.x, settingsRect.y + settingsOffsetY, settingsRect.w, settingsRect.h, sc.r, sc.g, sc.b, 1.0f);
    DrawPanelPx(quitRect.x, quitRect.y + quitOffsetY, quitRect.w, quitRect.h, qc.r, qc.g, qc.b, 1.0f);

    const char* playText = "PLAY";
    const char* settingsText = "SETTINGS";
    const char* quitText = "QUIT";

    // FIX: text is centered horizontally on screen (which matches the centered buttons)
    const float playTextX = GetCenteredX(playText, kBaseTextScale);
    const float settingsTextX = GetCenteredX(settingsText, kBaseTextScale);
    const float quitTextX = GetCenteredX(quitText, kBaseTextScale);

    // FIX: baseline inset now uses kTextBaselineInsetY = kButtonHeight * 0.58f for vertical centering
    const float playTextY = playRect.y + kTextBaselineInsetY + playOffsetY;
    const float settingsTextY = settingsRect.y + kTextBaselineInsetY + settingsOffsetY;
    const float quitTextY = quitRect.y + kTextBaselineInsetY + quitOffsetY;

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

    // Footer
    constexpr float kFooterScale = 0.7f;
    constexpr float kFooterMarginX = 12.0f;
    constexpr float kFooterMarginY = 22.0f;

    const float footerScreenW = (float)AEGfxGetWindowWidth();
    const float footerScreenH = (float)AEGfxGetWindowHeight();
    const float footerY = footerScreenH - kFooterMarginY;

    Print("MERGE DEFENDERS v1.0", kFooterMarginX, footerY, kInfo, kFooterScale);

    // FIX: clamp copyright so it never overflows either edge of the screen
    const char* copyright = "Copyright 2024 DigiPen Institute of Technology";
    const float copyrightW = (float)std::strlen(copyright) * kFooterCharWidth * kFooterScale;
    const float copyrightX = footerScreenW - copyrightW - kFooterMarginX;
    const float copyrightXClamped = (copyrightX < kFooterMarginX) ? kFooterMarginX : copyrightX;
    const float copyrightXFinal = (copyrightXClamped + copyrightW > footerScreenW - kFooterMarginX)
        ? footerScreenW - copyrightW - kFooterMarginX
        : copyrightXClamped;
    Print(copyright, copyrightXFinal, footerY, kInfo, kFooterScale);
}

#pragma endregion