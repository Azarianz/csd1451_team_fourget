#pragma once
#include "Scene.h"
#include "AEEngine.h"

class Scene_Settings : public Scene
{
public:
    void Init()           override;
    void Update(float dt) override;
    void Draw()           override;
    void Exit()           override;

private:
    enum class SettingRow { Resolution = 0, Volume, COUNT };

    SettingRow m_selected = SettingRow::Resolution;
    int        m_uiFont = -1;

    // Prevent held-key repeat from firing every frame
    float m_inputCooldown = 0.0f;
    static const float INPUT_REPEAT_DELAY;

    // Helpers
    void HandleInput(float dt);
    void HandleMouseInput();
    void DrawUI()      const;
    void ApplyVolume() const;

    float ScreenToNormX(float px)             const;
    float ScreenToNormY(float py)             const;
    float GetCenteredX(const char* text, float scale) const;
};