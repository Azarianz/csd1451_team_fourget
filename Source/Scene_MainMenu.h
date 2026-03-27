#pragma once

#include "Scene.h"

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

    void UpdateMouseInput();
    void MoveUp();
    void MoveDown();
    void ConfirmSelection();

    void HandlePlay();
    void HandleSettings();
    void HandleQuit();

    float ScreenToNormX(float px) const;
    float ScreenToNormY(float py) const;
    float GetCenteredX(const char* text, float scale) const;
    bool IsPointInRect(float mx, float my, float x, float y, float w, float h) const;

    ButtonRect GetPlayButtonRect() const;
    ButtonRect GetSettingsButtonRect() const;
    ButtonRect GetQuitButtonRect() const;

    void DrawUI() const;
    void DrawPanelPx(float x, float y, float w, float h,
        float r, float g, float b, float a) const;
};