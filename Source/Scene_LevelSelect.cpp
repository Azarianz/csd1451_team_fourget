// Scene_LevelSelect.cpp
#include "Scene_LevelSelect.h"
#include "Scene_LevelEditor.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "GameSettings.h"
#include "AEInput.h"
#include <algorithm>
#include <cstring>
#include <Windows.h>

SceneID Scene_LevelSelect::s_returnScene = SceneID::MainMenu;

void Scene_LevelSelect::SetReturnScene(SceneID id)
{
    s_returnScene = id;
}

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
    return (mx >= x && mx <= x + w && my >= y && my <= y + h);
}

int Scene_LevelSelect::GetLevelsPerPage() const
{
    return 4; // 2 columns x 2 rows
}

int Scene_LevelSelect::GetPageCount() const
{
    if (m_buttons.empty())
        return 1;

    if ((int)m_buttons.size() <= GetLevelsPerPage())
        return 1;

    return ((int)m_buttons.size() + GetLevelsPerPage() - 1) / GetLevelsPerPage();
}

int Scene_LevelSelect::GetCurrentPageStartIndex() const
{
    if ((int)m_buttons.size() <= GetLevelsPerPage())
        return 0;

    return m_currentPage * GetLevelsPerPage();
}

int Scene_LevelSelect::GetCurrentPageEndIndex() const
{
    if ((int)m_buttons.size() <= GetLevelsPerPage())
        return (int)m_buttons.size();

    int endIndex = GetCurrentPageStartIndex() + GetLevelsPerPage();
    if (endIndex > (int)m_buttons.size())
        endIndex = (int)m_buttons.size();
    return endIndex;
}

void Scene_LevelSelect::PrevPage()
{
    if (GetPageCount() <= 1)
        return;

    if (m_currentPage > 0)
        --m_currentPage;
    else
        m_currentPage = GetPageCount() - 1;

    m_selectedIndex = GetCurrentPageStartIndex();
}

void Scene_LevelSelect::NextPage()
{
    if (GetPageCount() <= 1)
        return;

    if (m_currentPage < GetPageCount() - 1)
        ++m_currentPage;
    else
        m_currentPage = 0;

    m_selectedIndex = GetCurrentPageStartIndex();
}

void Scene_LevelSelect::Init()
{
    m_selectedIndex = 0;
    m_currentPage = 0;
    m_state = MenuState::SelectingLevel;
    m_pendingLevelIndex = -1;
    m_selectedDifficulty = 0;
    m_columns = 2; // 2 columns

    if (m_uiFont < 0)
        m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    m_imageQuad = AEGfxMeshEnd();

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

    if (m_imageQuad)
    {
        AEGfxMeshFree(m_imageQuad);
        m_imageQuad = nullptr;
    }

    for (auto& b : m_buttons)
    {
        if (b.thumbnail)
            AEGfxTextureUnload(b.thumbnail);
    }
    m_buttons.clear();
}

void Scene_LevelSelect::LoadLevelList()
{
    m_buttons.clear();

    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile("Assets\\Levels\\*.txt", &findFileData);

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

        // Try to load thumbnail matching the level name
        std::string imgPath = "Assets/" + b.displayName + ".png";
        b.thumbnail = AEGfxTextureLoad(imgPath.c_str());

        if (!b.thumbnail)
        {
            imgPath = "Assets/Levels/" + b.displayName + ".png";
            b.thumbnail = AEGfxTextureLoad(imgPath.c_str());
        }

        m_buttons.push_back(b);

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    std::sort(m_buttons.begin(), m_buttons.end(),
        [](const LevelButton& a, const LevelButton& b)
        {
            return a.displayName < b.displayName;
        });
}

void Scene_LevelSelect::BuildButtonLayout()
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    // 4 stages per page max = 2 x 2 grid
    const float buttonH = screenH * 0.26f;
    const float buttonW = buttonH * (16.0f / 9.0f);

    const float spacingX = screenW * 0.05f;
    const float spacingY = buttonH * 0.22f;

    float totalGridW = m_columns * buttonW + (m_columns - 1) * spacingX;
    float startX = (screenW - totalGridW) * 0.5f;
    float startY = screenH * 0.20f;

    const int perPage = GetLevelsPerPage();

    for (size_t i = 0; i < m_buttons.size(); ++i)
    {
        int localIndex = (int)i % perPage;
        int col = localIndex % m_columns;
        int row = localIndex / m_columns;

        LevelButton& b = m_buttons[i];
        b.w = buttonW;
        b.h = buttonH;
        b.x = startX + col * (buttonW + spacingX);
        b.y = startY + row * (buttonH + spacingY);
    }
}

void Scene_LevelSelect::Update(float /*dt*/)
{
    UpdateMouseInput();

    if (m_state == MenuState::SelectingLevel)
    {
        if (AEInputCheckTriggered(AEVK_F5))
        {
            Scene_LevelEditor::SetReturnScene(SceneID::LevelSelect);
            SceneManager::I().SwitchTo(SceneID::LevelEditor);
            return;
        }

        if (m_buttons.empty())
        {
            if (AEInputCheckTriggered(AEVK_ESCAPE))
                SceneManager::I().SwitchTo(s_returnScene);
            return;
        }

        if (GetPageCount() > 1)
        {
            if (AEInputCheckTriggered(AEVK_Q))
                PrevPage();
            if (AEInputCheckTriggered(AEVK_E))
                NextPage();
        }

        if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_UP))
            MoveUp();
        if (AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_DOWN))
            MoveDown();
        if (AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT))
            MoveLeft();
        if (AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT))
            MoveRight();

        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
            SelectLevel((size_t)m_selectedIndex);

        if (AEInputCheckTriggered(AEVK_ESCAPE))
            SceneManager::I().SwitchTo(s_returnScene);
    }
    else if (m_state == MenuState::SelectingDifficulty)
    {
        if (AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT) ||
            AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT))
        {
            m_selectedDifficulty = (m_selectedDifficulty == 0) ? 1 : 0;
        }

        if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
        {
            ConfirmSelection();
        }

        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            m_state = MenuState::SelectingLevel;
            m_pendingLevelIndex = -1;
        }
    }
}

void Scene_LevelSelect::UpdateMouseInput()
{
    int mouseX = 0, mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    if (m_state == MenuState::SelectingLevel)
    {
        int startIndex = GetCurrentPageStartIndex();
        int endIndex = GetCurrentPageEndIndex();

        for (int i = startIndex; i < endIndex; ++i)
        {
            const LevelButton& b = m_buttons[(size_t)i];

            if (IsPointInRect((float)mouseX, (float)mouseY, b.x, b.y, b.w, b.h + 45.0f))
            {
                m_selectedIndex = i;

                if (AEInputCheckTriggered(AEVK_LBUTTON))
                {
                    SelectLevel((size_t)i);
                    return;
                }
            }
        }
    }
    else if (m_state == MenuState::SelectingDifficulty)
    {
        const float screenW = (float)AEGfxGetWindowWidth();
        const float screenH = (float)AEGfxGetWindowHeight();
        const float popY = screenH * 0.5f;

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

    int startIndex = GetCurrentPageStartIndex();
    int endIndex = GetCurrentPageEndIndex();
    int countOnPage = endIndex - startIndex;
    int localIndex = m_selectedIndex - startIndex;

    localIndex -= m_columns;
    if (localIndex < 0)
    {
        localIndex += countOnPage;
        if (localIndex >= countOnPage)
            localIndex = countOnPage - 1;
    }

    m_selectedIndex = startIndex + localIndex;
}

void Scene_LevelSelect::MoveDown()
{
    if (m_buttons.empty()) return;

    int startIndex = GetCurrentPageStartIndex();
    int endIndex = GetCurrentPageEndIndex();
    int countOnPage = endIndex - startIndex;
    int localIndex = m_selectedIndex - startIndex;

    localIndex += m_columns;
    if (localIndex >= countOnPage)
        localIndex %= m_columns;

    if (localIndex >= countOnPage)
        localIndex = countOnPage - 1;

    m_selectedIndex = startIndex + localIndex;
}

void Scene_LevelSelect::MoveLeft()
{
    if (m_buttons.empty()) return;

    int startIndex = GetCurrentPageStartIndex();
    int endIndex = GetCurrentPageEndIndex();
    int countOnPage = endIndex - startIndex;
    int localIndex = m_selectedIndex - startIndex;

    --localIndex;
    if (localIndex < 0)
        localIndex = countOnPage - 1;

    m_selectedIndex = startIndex + localIndex;
}

void Scene_LevelSelect::MoveRight()
{
    if (m_buttons.empty()) return;

    int startIndex = GetCurrentPageStartIndex();
    int endIndex = GetCurrentPageEndIndex();
    int countOnPage = endIndex - startIndex;
    int localIndex = m_selectedIndex - startIndex;

    ++localIndex;
    if (localIndex >= countOnPage)
        localIndex = 0;

    m_selectedIndex = startIndex + localIndex;
}

void Scene_LevelSelect::SelectLevel(size_t index)
{
    if (index >= m_buttons.size()) return;
    GameSettings::selectedLevelFile = "Assets/Levels/" + m_buttons[index].fileName;
    m_pendingLevelIndex = (int)index;
    m_state = MenuState::SelectingDifficulty;
    m_selectedDifficulty = 0;
}

void Scene_LevelSelect::ConfirmSelection()
{
    if (m_pendingLevelIndex < 0 || m_pendingLevelIndex >= (int)m_buttons.size()) return;
    GameSettings::currentDifficulty = (m_selectedDifficulty == 0) ? GameSettings::Difficulty::Easy : GameSettings::Difficulty::Hard;
    GameSettings::selectedLevelFile = "Assets/Levels/" + m_buttons[(size_t)m_pendingLevelIndex].fileName;
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
    const float titleY = screenH * 0.10f;
    const float infoY = screenH * 0.16f;
    const float bright = 1.0f;
    const float dim = 0.35f;
    const float info = 0.7f;

    auto Print = [&](const char* text, float px, float py, float shade, float scale = 1.0f)
        {
            AEGfxPrint(m_uiFont, text, ScreenToNormX(px), ScreenToNormY(py), scale, shade, shade, shade, 1.0f);
        };

    const char* title = "LEVEL SELECT";
    Print(title, GetCenteredX(title, 1.3f), titleY, bright, 1.3f);

    if (m_buttons.empty())
    {
        const char* none = "NO LEVEL TXT FILES FOUND IN Assets/Levels";
        Print(none, GetCenteredX(none, 0.9f), screenH * 0.42f, info, 0.9f);
        const char* back = "ESC - BACK    F5 - LEVEL EDITOR";
        Print(back, GetCenteredX(back, 0.8f), screenH * 0.52f, info, 0.8f);
        return;
    }

    const char* controls = "WASD/ARROWS - MOVE    ENTER - SELECT    F5 - LEVEL EDITOR    ESC - BACK";
    Print(controls, GetCenteredX(controls, 0.60f), infoY, info, 0.60f);

    if (GetPageCount() > 1)
    {
        const char* pageControls = "Q/E - CHANGE PAGE";
        Print(pageControls, GetCenteredX(pageControls, 0.55f), screenH * 0.92f, info, 0.55f);

        char pageText[32];
        sprintf_s(pageText, "PAGE %d / %d", m_currentPage + 1, GetPageCount());
        Print(pageText, GetCenteredX(pageText, 0.65f), screenH * 0.87f, bright, 0.65f);
    }

    int startIndex = GetCurrentPageStartIndex();
    int endIndex = GetCurrentPageEndIndex();

    // Draw Grid
    for (int i = startIndex; i < endIndex; ++i)
    {
        const LevelButton& b = m_buttons[(size_t)i];

        float colorShade = dim;
        if (m_state == MenuState::SelectingLevel)
            colorShade = (i == m_selectedIndex) ? bright : dim;
        else
            colorShade = 0.2f;

        // Border / Selection Highlight
        if (i == m_selectedIndex && m_state == MenuState::SelectingLevel)
        {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetColorToMultiply(0.8f, 0.8f, 0.2f, 1.0f); // Yellowish highlight

            AEMtx33 scaleM, transM, finalMtx;
            AEMtx33Scale(&scaleM, b.w + 10.0f, b.h + 10.0f);
            float cx = b.x + b.w * 0.5f;
            float cy = b.y + b.h * 0.5f;
            AEMtx33Trans(&transM, cx - screenW * 0.5f, screenH * 0.5f - cy);
            AEMtx33Concat(&finalMtx, &transM, &scaleM);
            AEGfxSetTransform(finalMtx.m);
            AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
        }

        // Draw Image Thumbnail
        if (b.thumbnail && m_imageQuad)
        {
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxTextureSet(b.thumbnail, 0, 0);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetTransparency(1.0f);
            AEGfxSetColorToMultiply(colorShade, colorShade, colorShade, 1.0f);

            AEMtx33 scale, trans, finalMtx;
            AEMtx33Scale(&scale, b.w, b.h);
            float cx = b.x + b.w * 0.5f;
            float cy = b.y + b.h * 0.5f;
            AEMtx33Trans(&trans, cx - screenW * 0.5f, screenH * 0.5f - cy);
            AEMtx33Concat(&finalMtx, &trans, &scale);
            AEGfxSetTransform(finalMtx.m);
            AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
        }
        else
        {
            // Placeholder rect if image missing
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetColorToMultiply(0.2f, 0.2f, 0.2f, 1.0f);
            AEMtx33 scale, trans, finalMtx;
            AEMtx33Scale(&scale, b.w, b.h);
            float cx = b.x + b.w * 0.5f;
            float cy = b.y + b.h * 0.5f;
            AEMtx33Trans(&trans, cx - screenW * 0.5f, screenH * 0.5f - cy);
            AEMtx33Concat(&finalMtx, &trans, &scale);
            AEGfxSetTransform(finalMtx.m);
            AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
        }

        // Display Name text below image
        float textScale = (screenW > 1500.0f) ? 1.2f : 1.0f;
        float textX = b.x + b.w * 0.5f - ((float)std::strlen(b.displayName.c_str()) * 22.0f * textScale * 0.5f);

        Print(b.displayName.c_str(), textX, b.y + b.h + 35.0f, colorShade, textScale);
    }

    if (m_state == MenuState::SelectingDifficulty)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(0.85f);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);

        AEMtx33 scaleM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Trans(&transM, 0, 0);
        AEMtx33Concat(&finalMtx, &transM, &scaleM);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(m_imageQuad, AE_GFX_MDM_TRIANGLES);
        AEGfxSetTransparency(1.0f);

        const char* popTitle = "SELECT DIFFICULTY";
        float popTitleY = screenH * 0.4f;
        Print(popTitle, GetCenteredX(popTitle, 1.2f), popTitleY, bright, 1.2f);

        float easyX = screenW * 0.4f;
        float hardX = screenW * 0.6f;
        float popY = screenH * 0.5f;

        Print("EASY", easyX - 22.0f, popY, m_selectedDifficulty == 0 ? bright : dim, 1.0f);
        if (m_selectedDifficulty == 0) Print(">", easyX - 56.0f, popY, bright, 1.0f);

        Print("HARD", hardX - 22.0f, popY, m_selectedDifficulty == 1 ? bright : dim, 1.0f);
        if (m_selectedDifficulty == 1) Print(">", hardX - 56.0f, popY, bright, 1.0f);

        const char* popControls = "A/D - SELECT    ENTER - CONFIRM    ESC - CANCEL";
        Print(popControls, GetCenteredX(popControls, 0.7f), screenH * 0.65f, info, 0.7f);
    }
}