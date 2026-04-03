#pragma once

#include "Scene.h"
#include "AEEngine.h"

class Scene_MainMenu : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    enum class MenuOption
    {
        Play,
        Settings,
        Controls,
        Credits,
        Quit
    };

    struct ButtonRect
    {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
    };

    MenuOption selectedOption = MenuOption::Play;
    int m_uiFont = -1;
    float m_bounceTimer = 0.0f;

    // Background music
    AEAudio      m_bgm;
    AEAudioGroup m_bgmGroup;
    bool         m_bgmLoaded = false;

    // Main menu background
    AEGfxTexture* m_bgTex = nullptr;

    void UpdateMouseInput();
    void MoveUp();
    void MoveDown();
    void ConfirmSelection();

    void HandlePlay();
    void HandleSettings();
    void HandleQuit();
    void HandleControls();
    void HandleCredits();

    float ScreenToNormX(float px) const;
    float ScreenToNormY(float py) const;
    float GetCenteredX(const char* text, float scale) const;
    bool IsPointInRect(float mx, float my, float x, float y, float w, float h) const;

    ButtonRect GetPlayButtonRect() const;
    ButtonRect GetSettingsButtonRect() const;
    ButtonRect GetQuitButtonRect() const;
    ButtonRect GetControlsButtonRect() const;
    ButtonRect GetCreditsButtonRect() const;

    void DrawUI() const;
    void DrawPanelPx(float x, float y, float w, float h,
        float r, float g, float b, float a) const;
    void DrawBackground() const;
};