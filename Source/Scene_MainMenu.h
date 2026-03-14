#pragma once
#include "Scene.h"

class Scene_MainMenu : public Scene
{
public:
    void Init()   override;
    void Update(float dt) override;
    void Draw()   override;
    void Exit()   override;

private:
    enum class MenuOption { Play = 0, Settings, Quit };

    MenuOption selectedOption = MenuOption::Play;
    int        m_uiFont = -1;

    void MoveUp();
    void MoveDown();
    void ConfirmSelection();

    void HandlePlay();
    void HandleSettings();
    void HandleQuit();

    void DrawUI() const;
    void UpdateMouseInput();

    float ScreenToNormX(float px) const;
    float ScreenToNormY(float py) const;
    float GetCenteredX(const char* text, float scale) const;
    bool  IsPointInRect(float mx, float my, float x, float y, float w, float h) const;

    struct ButtonRect { float x, y, w, h; };
    ButtonRect GetPlayButtonRect()     const;
    ButtonRect GetSettingsButtonRect() const;
    ButtonRect GetQuitButtonRect()     const;
};