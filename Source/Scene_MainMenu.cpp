#include "Scene_MainMenu.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include <cstring>

float Scene_MainMenu::ScreenToNormX(float px) const
{
    return (px / AEGfxGetWindowWidth()) * 2.0f - 1.0f;
}

float Scene_MainMenu::ScreenToNormY(float py) const
{
    return 1.0f - (py / AEGfxGetWindowHeight()) * 2.0f;
}

void Scene_MainMenu::Init()
{
    selectedOption = MenuOption::Play;
    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 24);
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

void Scene_MainMenu::UpdateMouseInput()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    ButtonRect play = GetPlayButtonRect();
    ButtonRect quit = GetQuitButtonRect();

    if (IsPointInRect((float)mouseX, (float)mouseY, play.x, play.y, play.w, play.h))
    {
        selectedOption = MenuOption::Play;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandlePlay();
    }
    else if (IsPointInRect((float)mouseX, (float)mouseY, quit.x, quit.y, quit.w, quit.h))
    {
        selectedOption = MenuOption::Quit;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
            HandleQuit();
    }
}

void Scene_MainMenu::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);
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

void Scene_MainMenu::MoveUp()
{
    if (selectedOption == MenuOption::Play)
        selectedOption = MenuOption::Quit;
    else
        selectedOption = MenuOption::Play;
}

void Scene_MainMenu::MoveDown()
{
    if (selectedOption == MenuOption::Play)
        selectedOption = MenuOption::Quit;
    else
        selectedOption = MenuOption::Play;
}

void Scene_MainMenu::ConfirmSelection()
{
    switch (selectedOption)
    {
    case MenuOption::Play:
        HandlePlay();
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
    SceneManager::I().SwitchTo(SceneID::Prototype);
}

void Scene_MainMenu::HandleQuit()
{
    AESysExit();
}

float Scene_MainMenu::GetCenteredX(const char* text, float scale) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float centerX = screenW * 0.5f;

    // Approximate width per character in pixels for this font.
    // Tweak this if needed.
    const float charWidthPx = 22.f * scale;

    const float textWidth = (float)std::strlen(text) * charWidthPx;
    return centerX - textWidth * 0.5f;
}

bool Scene_MainMenu::IsPointInRect(float mx, float my, float x, float y, float w, float h) const
{
    return (mx >= x && mx <= x + w &&
        my >= y && my <= y + h);
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetPlayButtonRect() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect r;
    r.w = 220.0f;
    r.h = 42.0f;
    r.x = screenW * 0.5f - r.w * 0.5f;
    r.y = screenH * 0.42f - r.h * 0.5f;
    return r;
}

Scene_MainMenu::ButtonRect Scene_MainMenu::GetQuitButtonRect() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    ButtonRect r;
    r.w = 220.0f;
    r.h = 42.0f;
    r.x = screenW * 0.5f - r.w * 0.5f;
    r.y = screenH * 0.42f + 45.0f - r.h * 0.5f;
    return r;
}

void Scene_MainMenu::DrawUI() const
{
    if (m_uiFont < 0) return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();
    const float centerX = screenW * 0.5f;

    const float titleY = screenH * 0.28f;
    const float playY = screenH * 0.42f;
    const float quitY = playY + 45.0f;
    const float controlsY = screenH * 0.60f;

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

    // Title
    const char* title = "MERGE DEFENDERS";
    Print(title, GetCenteredX(title, 1.4f), titleY, bright, 1.4f);

    // Buttons
    const char* playText = "PLAY";
    const char* quitText = "QUIT";

    float playX = GetCenteredX(playText, 1.0f);
    float quitX = GetCenteredX(quitText, 1.0f);

    Print(playText, playX, playY,
        selectedOption == MenuOption::Play ? bright : dim, 1.0f);

    Print(quitText, quitX, quitY,
        selectedOption == MenuOption::Quit ? bright : dim, 1.0f);

    // Arrow
    const float arrowOffset = 34.0f;
    if (selectedOption == MenuOption::Play)
        Print(">", playX - arrowOffset, playY, bright, 1.0f);
    else
        Print(">", quitX - arrowOffset, quitY, bright, 1.0f);

    // Controls
    const char* controls = "CONTROLS:";
    float controlsX = GetCenteredX(controls, 0.9f);
    Print(controls, controlsX, controlsY, info, 0.9f);

    const char* line1 = "W / UP    - Move";
    const char* line2 = "S / DOWN  - Move";
    const char* line3 = "ENTER     - Select";
    //const char* line4 = "ESC       - Quit";

    float y = controlsY + 35.0f;
    Print(line1, GetCenteredX(line1, 0.8f), y, info, 0.8f);

    y += 28.0f;
    Print(line2, GetCenteredX(line2, 0.8f), y, info, 0.8f);

    y += 28.0f;
    Print(line3, GetCenteredX(line3, 0.8f), y, info, 0.8f);

    //y += 28.0f;
    //Print(line4, GetCenteredX(line4, 0.8f), y, info, 0.8f);
}