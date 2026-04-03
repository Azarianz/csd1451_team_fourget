#include "Scene_MainMenu.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include "GameSettings.h"
#include "Scene_LevelSelect.h"

#include <cstring>
#include <cmath>

namespace
{
    constexpr float kBgR = 0.08f;
    constexpr float kBgG = 0.08f;
    constexpr float kBgB = 0.12f;

    constexpr float kBaseTextScale = 1.0f;
    constexpr float kTitleScale = 1.85f;
    constexpr float kControlsScale = 0.9f;
    constexpr float kControlsLineScale = 0.8f;

    constexpr float kApproxCharWidth = 22.0f;

    constexpr float kTitleYRatio = 0.15f;
    constexpr float kControlsYRatio = 0.68f;

    // Fixed menu layout
    constexpr float kButtonStartY = 180.0f;
    constexpr float kButtonSpacing = 90.0f;
    constexpr float kButtonHeight = 82.0f;
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

    if (!m_bgTex)
    {
        m_bgTex = AEGfxTextureLoad("Assets/mainmenu_bg.png");
    }

    m_bgmLoaded = true;
    float normalised = GameSettings::masterVolume / 100.0f;
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
    DrawBackground();
    DrawUI();
}

void Scene_MainMenu::Exit()
{
    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    if (m_bgTex)
    {
        AEGfxTextureUnload(m_bgTex);
        m_bgTex = nullptr;
    }

    if (m_bgmLoaded)
    {
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
    const ButtonRect controlsRect = GetControlsButtonRect();
    const ButtonRect creditsRect = GetCreditsButtonRect();
    const ButtonRect quitRect = GetQuitButtonRect();

    if (IsPointInRect((float)mouseX, (float)mouseY, playRect.x, playRect.y, playRect.w, playRect.h))
    {
        selectedOption = MenuOption::Play;
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandlePlay();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, controlsRect.x, controlsRect.y, controlsRect.w, controlsRect.h))
    {
        selectedOption = MenuOption::Controls;
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandleControls();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, creditsRect.x, creditsRect.y, creditsRect.w, creditsRect.h))
    {
        selectedOption = MenuOption::Credits;
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandleCredits();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, settingsRect.x, settingsRect.y, settingsRect.w, settingsRect.h))
    {
        selectedOption = MenuOption::Settings;
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandleSettings();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, quitRect.x, quitRect.y, quitRect.w, quitRect.h))
    {
        selectedOption = MenuOption::Quit;
        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandleQuit();
    }
}

void Scene_MainMenu::MoveUp()
{
    switch (selectedOption)
    {
    case MenuOption::Play:     selectedOption = MenuOption::Quit;     break;
    case MenuOption::Controls: selectedOption = MenuOption::Play;     break;
    case MenuOption::Credits:  selectedOption = MenuOption::Controls; break;
    case MenuOption::Settings: selectedOption = MenuOption::Credits;  break;
    case MenuOption::Quit:     selectedOption = MenuOption::Settings; break;
    }
}

void Scene_MainMenu::MoveDown()
{
    switch (selectedOption)
    {
    case MenuOption::Play:     selectedOption = MenuOption::Controls; break;
    case MenuOption::Controls: selectedOption = MenuOption::Credits;  break;
    case MenuOption::Credits:  selectedOption = MenuOption::Settings; break;
    case MenuOption::Settings: selectedOption = MenuOption::Quit;     break;
    case MenuOption::Quit:     selectedOption = MenuOption::Play;     break;
    }
}

void Scene_MainMenu::ConfirmSelection()
{
    switch (selectedOption)
    {
    case MenuOption::Play:     HandlePlay();     break;
    case MenuOption::Settings: HandleSettings(); break;
    case MenuOption::Controls: HandleControls(); break;
    case MenuOption::Credits:  HandleCredits();  break;
    case MenuOption::Quit:     HandleQuit();     break;
    default: break;
    }
}

void Scene_MainMenu::HandlePlay()
{
    Scene_LevelSelect::SetReturnScene(SceneID::MainMenu);
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
void Scene_MainMenu::HandleControls()
{
    SceneManager::I().SwitchTo(SceneID::Controls);
}

void Scene_MainMenu::HandleCredits()
{
    SceneManager::I().SwitchTo(SceneID::Credits);
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
    const char* widestText = "CONTROLS";
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
    const char* widestText = "CONTROLS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY + (kButtonSpacing * 3.0f); // moved down to 4th button
    return rect;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetQuitButtonRect() const
{
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "CONTROLS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY + (kButtonSpacing * 4.0f);
    return rect;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetControlsButtonRect() const
{
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "CONTROLS";
    const float textW = (float)std::strlen(widestText) * kApproxCharWidth;
    const float screenW = (float)AEGfxGetWindowWidth();

    ButtonRect rect;
    rect.w = textW + (kButtonPadX * 2.0f);
    rect.h = kButtonHeight;
    rect.x = screenW * 0.5f - rect.w * 0.5f;
    rect.y = kButtonStartY + (kButtonSpacing * 1.0f);
    return rect;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetCreditsButtonRect() const
{
    // FIX: all buttons use "SETTINGS" (widest label) for width so all three are equal
    const char* widestText = "CONTROLS";
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

void Scene_MainMenu::DrawBackground() const
{
    if (!m_bgTex)
        return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_bgTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);

    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

    AEGfxVertexList* mesh = AEGfxMeshEnd();

    if (mesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, 0.0f, 0.0f);

        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);

        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(mesh);
    }
}

#pragma endregion

#pragma region UIDrawing

void Scene_MainMenu::DrawUI() const
{
    if (m_uiFont < 0)
        return;

    const float screenH = (float)AEGfxGetWindowHeight();
    const float titleY = screenH * kTitleYRatio;

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
    const ButtonRect controlsRect = GetControlsButtonRect();
    const ButtonRect creditsRect = GetCreditsButtonRect();
    const ButtonRect settingsRect = GetSettingsButtonRect();
    const ButtonRect quitRect = GetQuitButtonRect();

    const bool playHover =
        IsPointInRect((float)mouseX, (float)mouseY, playRect.x, playRect.y, playRect.w, playRect.h);
    const bool controlsHover =
        IsPointInRect((float)mouseX, (float)mouseY, controlsRect.x, controlsRect.y, controlsRect.w, controlsRect.h);
    const bool creditsHover =
        IsPointInRect((float)mouseX, (float)mouseY, creditsRect.x, creditsRect.y, creditsRect.w, creditsRect.h);
    const bool settingsHover =
        IsPointInRect((float)mouseX, (float)mouseY, settingsRect.x, settingsRect.y, settingsRect.w, settingsRect.h);
    const bool quitHover =
        IsPointInRect((float)mouseX, (float)mouseY, quitRect.x, quitRect.y, quitRect.w, quitRect.h);

    Print("MERGE DEFENDERS", GetCenteredX("MERGE DEFENDERS", kTitleScale), titleY, kBright, kTitleScale);

    const float bounceOffset = sinf(m_bounceTimer * kBounceSpeed) * kBounceAmplitude;

    const float playOffsetY = (selectedOption == MenuOption::Play) ? -bounceOffset : 0.0f;
    const float controlsOffsetY = (selectedOption == MenuOption::Controls) ? -bounceOffset : 0.0f;
    const float creditsOffsetY = (selectedOption == MenuOption::Credits) ? -bounceOffset : 0.0f;
    const float settingsOffsetY = (selectedOption == MenuOption::Settings) ? -bounceOffset : 0.0f;
    const float quitOffsetY = (selectedOption == MenuOption::Quit) ? -bounceOffset : 0.0f;

    struct RGB { float r, g, b; };
    auto PanelColour = [&](MenuOption opt, bool hover) -> RGB
        {
            if (selectedOption == opt)
                return { 0.12f, 0.20f, 0.58f };
            if (hover)
                return { 0.35f, 0.35f, 0.42f };
            return { 0.12f, 0.12f, 0.18f };
        };

    RGB pc = PanelColour(MenuOption::Play, playHover);
    RGB coc = PanelColour(MenuOption::Controls, controlsHover);
    RGB crc = PanelColour(MenuOption::Credits, creditsHover);
    RGB sc = PanelColour(MenuOption::Settings, settingsHover);
    RGB qc = PanelColour(MenuOption::Quit, quitHover);

    DrawPanelPx(playRect.x, playRect.y + playOffsetY, playRect.w, playRect.h, pc.r, pc.g, pc.b, 1.0f);
    DrawPanelPx(controlsRect.x, controlsRect.y + controlsOffsetY, controlsRect.w, controlsRect.h, coc.r, coc.g, coc.b, 1.0f);
    DrawPanelPx(creditsRect.x, creditsRect.y + creditsOffsetY, creditsRect.w, creditsRect.h, crc.r, crc.g, crc.b, 1.0f);
    DrawPanelPx(settingsRect.x, settingsRect.y + settingsOffsetY, settingsRect.w, settingsRect.h, sc.r, sc.g, sc.b, 1.0f);
    DrawPanelPx(quitRect.x, quitRect.y + quitOffsetY, quitRect.w, quitRect.h, qc.r, qc.g, qc.b, 1.0f);

    const char* playText = "PLAY";
    const char* controlsText = "GUIDE";
    const char* creditsText = "CREDITS";
    const char* settingsText = "SETTINGS";
    const char* quitText = "QUIT";

    const float playTextX = GetCenteredX(playText, kBaseTextScale);
    const float controlsTextX = GetCenteredX(controlsText, kBaseTextScale);
    const float creditsTextX = GetCenteredX(creditsText, kBaseTextScale);
    const float settingsTextX = GetCenteredX(settingsText, kBaseTextScale);
    const float quitTextX = GetCenteredX(quitText, kBaseTextScale);

    const float playTextY = playRect.y + kTextBaselineInsetY + playOffsetY;
    const float controlsTextY = controlsRect.y + kTextBaselineInsetY + controlsOffsetY;
    const float creditsTextY = creditsRect.y + kTextBaselineInsetY + creditsOffsetY;
    const float settingsTextY = settingsRect.y + kTextBaselineInsetY + settingsOffsetY;
    const float quitTextY = quitRect.y + kTextBaselineInsetY + quitOffsetY;

    Print(playText, playTextX, playTextY,
        (selectedOption == MenuOption::Play || playHover) ? kBright : kDim, kBaseTextScale);

    Print(controlsText, controlsTextX, controlsTextY,
        (selectedOption == MenuOption::Controls || controlsHover) ? kBright : kDim, kBaseTextScale);

    Print(creditsText, creditsTextX, creditsTextY,
        (selectedOption == MenuOption::Credits || creditsHover) ? kBright : kDim, kBaseTextScale);

    Print(settingsText, settingsTextX, settingsTextY,
        (selectedOption == MenuOption::Settings || settingsHover) ? kBright : kDim, kBaseTextScale);

    Print(quitText, quitTextX, quitTextY,
        (selectedOption == MenuOption::Quit || quitHover) ? kBright : kDim, kBaseTextScale);

    if (selectedOption == MenuOption::Play)
        Print(">", playRect.x - kArrowOffsetX, playTextY, kBright, kBaseTextScale);
    else if (selectedOption == MenuOption::Controls)
        Print(">", controlsRect.x - kArrowOffsetX, controlsTextY, kBright, kBaseTextScale);
    else if (selectedOption == MenuOption::Credits)
        Print(">", creditsRect.x - kArrowOffsetX, creditsTextY, kBright, kBaseTextScale);
    else if (selectedOption == MenuOption::Settings)
        Print(">", settingsRect.x - kArrowOffsetX, settingsTextY, kBright, kBaseTextScale);
    else
        Print(">", quitRect.x - kArrowOffsetX, quitTextY, kBright, kBaseTextScale);

    constexpr float kFooterScale = 0.7f;
    constexpr float kFooterMarginX = 12.0f;
    constexpr float kFooterMarginY = 22.0f;

    const float footerScreenW = (float)AEGfxGetWindowWidth();
    const float footerScreenH = (float)AEGfxGetWindowHeight();
    const float footerY = footerScreenH - kFooterMarginY;

    //Print("MERGE DEFENDERS", kFooterMarginX, footerY, kInfo, kFooterScale);

    const char* copyright = "Copyright 2026 DigiPen Institute of Technology";
    const float copyrightW = (float)std::strlen(copyright) * kFooterCharWidth * kFooterScale;
    const float copyrightX = footerScreenW - copyrightW - kFooterMarginX;
    const float copyrightXClamped = (copyrightX < kFooterMarginX) ? kFooterMarginX : copyrightX;
    const float copyrightXFinal = (copyrightXClamped + copyrightW > footerScreenW - kFooterMarginX)
        ? footerScreenW - copyrightW - kFooterMarginX
        : copyrightXClamped;
    //Print(copyright, copyrightXFinal, footerY, kInfo, kFooterScale);
}

#pragma endregion