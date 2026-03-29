// Scene_LevelSelect.h
#pragma once
#include "Scene.h"
#include "AEEngine.h"
#include <vector>
#include <string>

class Scene_LevelSelect : public Scene
{
public:
    void Init() override;
    void Update(float dt) override;
    void Draw() override;
    void Exit() override;

private:
    struct LevelButton
    {
        std::string fileName;
        std::string displayName;
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        AEGfxTexture* thumbnail = nullptr;
    };

private:
    enum class MenuState { SelectingLevel, SelectingDifficulty };
    MenuState m_state = MenuState::SelectingLevel;

    int m_pendingLevelIndex = -1;
    int m_selectedDifficulty = 0;

    int m_columns = 2; // For grid layout

    void LoadLevelList();
    void BuildButtonLayout();
    void UpdateMouseInput();

    void MoveUp();
    void MoveDown();
    void MoveLeft();
    void MoveRight();
    void ConfirmSelection();
    void SelectLevel(size_t index);

    bool IsPointInRect(float mx, float my, float x, float y, float w, float h) const;
    float ScreenToNormX(float px) const;
    float ScreenToNormY(float py) const;
    float GetCenteredX(const char* text, float scale) const;

    void DrawUI() const;

private:
    std::vector<LevelButton> m_buttons;
    int m_selectedIndex = 0;
    s8 m_uiFont = -1;
    AEGfxVertexList* m_imageQuad = nullptr;
};