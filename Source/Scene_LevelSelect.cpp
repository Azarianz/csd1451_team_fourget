#include "Scene_LevelSelect.h"

#include "SceneManager.h"
#include "SceneID.h"
#include "GameSettings.h"
#include "AEInput.h"

#include <algorithm>
#include <cstring>

#include <Windows.h>

float Scene_LevelSelect::ScreenToNormX(float px) const
{
    return (px / AEGfxGetWindowWidth()) * 2.0f - 1.0f;
}

float Scene_LevelSelect::ScreenToNormY(float py) const
{
    return 1.0f - (py / AEGfxGetWindowHeight()) * 2.0f;
}

float Scene_LevelSelect::GetCenteredX(const char* text, float scale) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float centerX = screenW * 0.5f;

    const float charWidthPx = 22.0f * scale;
    const float textWidth = (float)std::strlen(text) * charWidthPx;
    return centerX - textWidth * 0.5f;
}

bool Scene_LevelSelect::IsPointInRect(float mx, float my, float x, float y, float w, float h) const
{
    return (mx >= x && mx <= x + w &&
        my >= y && my <= y + h);
}

void Scene_LevelSelect::Init()
{
    m_selectedIndex = 0;
    m_state = MenuState::SelectingLevel;
    m_pendingLevelIndex = -1;
    m_selectedDifficulty = 0;

    if (m_uiFont < 0)
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    LoadLevelList();
    BuildButtonLayout();
}

void Scene_LevelSelect::Exit()
{
    if (m_uiFont >= 0)
    {
        AEGfxDestroyFont(m_uiFont);
        m_uiFont = -1;
    }

    m_buttons.clear();
}

void Scene_LevelSelect::LoadLevelList()
{
    m_buttons.clear();

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;

    hFind = FindFirstFile("Assets\\Levels\\*.txt", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
        return;

    do
    {
        LevelButton b;

        b.fileName = findFileData.cFileName;

        std::string name = b.fileName;
        size_t dot = name.find_last_of('.');
        if (dot != std::string::npos)
            name = name.substr(0, dot);

        b.displayName = name;

        m_buttons.push_back(b);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void Scene_LevelSelect::BuildButtonLayout()
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float buttonW = 360.0f;
    const float buttonH = 42.0f;
    const float spacing = 50.0f;

    float startY = screenH * 0.32f;

    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        LevelButton& b = m_buttons[i];
        b.w = buttonW;
        b.h = buttonH;
        b.x = screenW * 0.5f - buttonW * 0.5f;
        b.y = startY + (float)i * spacing;
    }
}

void Scene_LevelSelect::Update(float /*dt*/)
{
    UpdateMouseInput();

    if (m_buttons.empty())
        return;

    if (m_state == MenuState::SelectingLevel)
    {
        if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_UP))
            MoveUp();

        if (AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_DOWN))
            MoveDown();

        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
            SelectLevel(m_selectedIndex);

        if (AEInputCheckTriggered(AEVK_ESCAPE))
            SceneManager::I().SwitchTo(SceneID::MainMenu);
    }
    else if (m_state == MenuState::SelectingDifficulty)
    {
        // Toggle difficulty with A/D or Left/Right
        if (AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT) ||
            AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT))
        {
            m_selectedDifficulty = (m_selectedDifficulty == 0) ? 1 : 0;
        }

        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
        {
            ConfirmSelection();
        }

        // Cancel and go back to level list
        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            m_state = MenuState::SelectingLevel;
            m_pendingLevelIndex = -1;
        }
    }
}

void Scene_LevelSelect::UpdateMouseInput()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    if (m_state == MenuState::SelectingLevel)
    {
        for (size_t i = 0; i < m_buttons.size(); ++i)
        {
            const LevelButton& b = m_buttons[i];

            if (IsPointInRect((float)mouseX, (float)mouseY, b.x, b.y, b.w, b.h))
            {
                m_selectedIndex = (int)i;

                if (AEInputCheckTriggered(AEVK_LBUTTON))
                {
                    SelectLevel(i);
                    return;
                }
            }
        }
    }
    else if (m_state == MenuState::SelectingDifficulty)
    {
        // Difficulty pop-up mouse logic
        const float screenW = (float)AEGfxGetWindowWidth();
        const float screenH = (float)AEGfxGetWindowHeight();
        const float popY = screenH * 0.5f;

        // Define click areas for Easy and Hard
        float easyX = screenW * 0.4f - 50.0f;
        float hardX = screenW * 0.6f - 50.0f;
        float btnW = 100.0f;
        float btnH = 40.0f;

        if (IsPointInRect((float)mouseX, (float)mouseY, easyX, popY - 20.0f, btnW, btnH))
        {
            m_selectedDifficulty = 0;
            if (AEInputCheckTriggered(AEVK_LBUTTON)) ConfirmSelection();
        }
        else if (IsPointInRect((float)mouseX, (float)mouseY, hardX, popY - 20.0f, btnW, btnH))
        {
            m_selectedDifficulty = 1;
            if (AEInputCheckTriggered(AEVK_LBUTTON)) ConfirmSelection();
        }
    }
}

void Scene_LevelSelect::MoveUp()
{
    if (m_buttons.empty()) return;

    --m_selectedIndex;
    if (m_selectedIndex < 0)
        m_selectedIndex = (int)m_buttons.size() - 1;
}

void Scene_LevelSelect::MoveDown()
{
    if (m_buttons.empty()) return;

    ++m_selectedIndex;
    if (m_selectedIndex >= (int)m_buttons.size())
        m_selectedIndex = 0;
}

void Scene_LevelSelect::SelectLevel(size_t index)
{
    if (index >= m_buttons.size())
        return;

    GameSettings::selectedLevelFile = "Assets/Levels/" + m_buttons[index].fileName;
    PRINT("Selected level file: %s\n", GameSettings::selectedLevelFile.c_str());
    // Open difficulty popup instead of immediately loading
    m_pendingLevelIndex = (int)index;
    m_state = MenuState::SelectingDifficulty;
    m_selectedDifficulty = 0; // Default to Easy
}

void Scene_LevelSelect::ConfirmSelection()
{
    if (m_pendingLevelIndex < 0 || m_pendingLevelIndex >= (int)m_buttons.size())
        return;

    // Apply the chosen difficulty to global settings
    GameSettings::currentDifficulty = (m_selectedDifficulty == 0)
        ? GameSettings::Difficulty::Easy
        : GameSettings::Difficulty::Hard;

    // Set level file and launch
    GameSettings::selectedLevelFile = "Assets/Levels/" + m_buttons[m_pendingLevelIndex].fileName;
    PRINT("Selected level file: %s (Difficulty: %d)\n", GameSettings::selectedLevelFile.c_str(), m_selectedDifficulty);

    SceneManager::I().SwitchTo(SceneID::Prototype);
}

void Scene_LevelSelect::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);
    DrawUI();
}

void Scene_LevelSelect::DrawUI() const
{
    if (m_uiFont < 0) return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();
    const float titleY = screenH * 0.18f;
    const float infoY = screenH * 0.24f;
    const float bright = 1.0f;
    const float dim = 0.35f;
    const float info = 0.7f;

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

    const char* title = "LEVEL SELECT";
    Print(title, GetCenteredX(title, 1.3f), titleY, bright, 1.3f);

    if (m_buttons.empty())
    {
        const char* none = "NO LEVEL TXT FILES FOUND IN Assets/Levels";
        Print(none, GetCenteredX(none, 0.9f), screenH * 0.42f, info, 0.9f);

        const char* back = "ESC - Back";
        Print(back, GetCenteredX(back, 0.8f), screenH * 0.52f, info, 0.8f);
        return;
    }

    const char* controls = "W/S OR UP/DOWN - MOVE    ENTER - SELECT    ESC - BACK";
    Print(controls, GetCenteredX(controls, 0.75f), infoY, info, 0.75f);

    const float arrowOffset = 34.0f;

    // Draw level list
    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        const LevelButton& b = m_buttons[i];

        // Dim the list heavily if the popup is open
        float colorShade = dim;
        if (m_state == MenuState::SelectingLevel) {
            colorShade = ((int)i == m_selectedIndex) ? bright : dim;
        }
        else {
            colorShade = 0.2f;
        }

        Print(
            b.displayName.c_str(),
            GetCenteredX(b.displayName.c_str(), 1.0f),
            b.y,
            colorShade,
            1.0f
        );

        if (m_state == MenuState::SelectingLevel && (int)i == m_selectedIndex)
        {
            float textX = GetCenteredX(b.displayName.c_str(), 1.0f);
            Print(">", textX - arrowOffset, b.y, bright, 1.0f);
        }
    }

    // Draw Pop-up Overlay
    if (m_state == MenuState::SelectingDifficulty)
    {
        // 1. Draw semi-transparent background box
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(0.85f);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);

        AEGfxMeshStart();
        AEGfxTriAdd(
            -1.0f, -1.0f, 0xFFFFFFFF, 0.0f, 0.0f,
            1.0f, -1.0f, 0xFFFFFFFF, 1.0f, 0.0f,
            1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 1.0f
        );
        AEGfxTriAdd(
            -1.0f, -1.0f, 0xFFFFFFFF, 0.0f, 0.0f,
            1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 1.0f,
            -1.0f, 1.0f, 0xFFFFFFFF, 0.0f, 1.0f
        );
        AEGfxVertexList* darkMesh = AEGfxMeshEnd();

        AEMtx33 identity;
        AEMtx33Identity(&identity);
        AEGfxSetTransform(identity.m);
        AEGfxMeshDraw(darkMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(darkMesh);
        AEGfxSetTransparency(1.0f);

        // 2. Draw Pop-up Text
        const char* popTitle = "SELECT DIFFICULTY";
        float popTitleY = screenH * 0.4f;
        Print(popTitle, GetCenteredX(popTitle, 1.2f), popTitleY, bright, 1.2f);

        float easyX = screenW * 0.4f;
        float hardX = screenW * 0.6f;
        float popY = screenH * 0.5f;

        // Easy Button
        Print("EASY", easyX - 22.0f, popY, m_selectedDifficulty == 0 ? bright : dim, 1.0f);
        if (m_selectedDifficulty == 0) Print(">", easyX - 22.0f - arrowOffset, popY, bright, 1.0f);

        // Hard Button
        Print("HARD", hardX - 22.0f, popY, m_selectedDifficulty == 1 ? bright : dim, 1.0f);
        if (m_selectedDifficulty == 1) Print(">", hardX - 22.0f - arrowOffset, popY, bright, 1.0f);

        const char* popControls = "A/D - SELECT    ENTER - CONFIRM    ESC - CANCEL";
        Print(popControls, GetCenteredX(popControls, 0.7f), screenH * 0.65f, info, 0.7f);
    }
}