#include "Scene_Settings.h"
#include "GameSettings.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "AEEngine.h"
#include "AEInput.h"
#include <cstring>
#include <cstdio>

const float Scene_Settings::INPUT_REPEAT_DELAY = 0.15f;

float Scene_Settings::ScreenToNormX(float px) const
{
    return (px / (float)AEGfxGetWindowWidth()) * 2.0f - 1.0f;
}

float Scene_Settings::ScreenToNormY(float py) const
{
    return 1.0f - (py / (float)AEGfxGetWindowHeight()) * 2.0f;
}

float Scene_Settings::GetCenteredX(const char* text, float scale) const
{
    const float charWidthPx = 22.f * scale;
    const float textWidth = (float)std::strlen(text) * charWidthPx;
    return (float)AEGfxGetWindowWidth() * 0.5f - textWidth * 0.5f;
}

void Scene_Settings::ApplyVolume() const
{
    if (!m_bgmLoaded) return;
    float normalised = GameSettings::masterVolume / 100.0f;
    AEAudioSetGroupVolume(m_bgmGroup, normalised);
}

// Init
void Scene_Settings::Init()
{
    m_selected = SettingRow::Resolution;
    m_inputCooldown = 0.0f;
    m_bgmLoaded = false;

    m_uiFont = AEGfxCreateFont("Assets/buggy-font.ttf", 28);

    // Load and play background music
    m_bgm = AEAudioLoadMusic("Assets/bouken.mp3");
    m_bgmGroup = AEAudioCreateGroup();

    m_bgmLoaded = true;
    AEAudioPlay(m_bgm, m_bgmGroup, 1.0f, 1.0f, -1);
    ApplyVolume();
}

void Scene_Settings::Exit()
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

// HandleInput
void Scene_Settings::HandleInput(float dt)
{
    if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_BACK))
    {
        SceneManager::I().SwitchTo(SceneID::MainMenu);
        return;
    }

    if (AEInputCheckTriggered(AEVK_W) || AEInputCheckTriggered(AEVK_UP))
    {
        int v = (int)m_selected - 1;
        if (v < 0) v = (int)SettingRow::COUNT - 1;
        m_selected = (SettingRow)v;
    }
    if (AEInputCheckTriggered(AEVK_S) || AEInputCheckTriggered(AEVK_DOWN))
    {
        int v = ((int)m_selected + 1) % (int)SettingRow::COUNT;
        m_selected = (SettingRow)v;
    }

    bool leftTrig = AEInputCheckTriggered(AEVK_A) || AEInputCheckTriggered(AEVK_LEFT);
    bool rightTrig = AEInputCheckTriggered(AEVK_D) || AEInputCheckTriggered(AEVK_RIGHT);
    bool leftHeld = AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT);
    bool rightHeld = AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT);

    bool canRepeat = (m_inputCooldown <= 0.0f);
    bool doLeft = leftTrig || (leftHeld && canRepeat && !leftTrig);
    bool doRight = rightTrig || (rightHeld && canRepeat && !rightTrig);

    if (doLeft || doRight)
    {
        m_inputCooldown = INPUT_REPEAT_DELAY;
        int delta = doRight ? 1 : -1;

        switch (m_selected)
        {
        case SettingRow::Resolution:
            GameSettings::resolutionIndex =
                (GameSettings::resolutionIndex + delta + GameSettings::RESOLUTION_COUNT)
                % GameSettings::RESOLUTION_COUNT;
            GameSettings::pendingRestart = true;
            GameSettings::Save();
            break;

        case SettingRow::Volume:
            GameSettings::masterVolume += delta * 5;
            if (GameSettings::masterVolume < 0)   GameSettings::masterVolume = 0;
            if (GameSettings::masterVolume > 100) GameSettings::masterVolume = 100;
            GameSettings::Save();
            ApplyVolume();
            break;

        default: break;
        }
    }

    if (m_inputCooldown > 0.0f)
        m_inputCooldown -= dt;
}

void Scene_Settings::Update(float dt)
{
    HandleInput(dt);
    HandleMouseInput();
}

void Scene_Settings::HandleMouseInput()
{
    int rawMX = 0, rawMY = 0;
    AEInputGetCursorPosition(&rawMX, &rawMY);
    const float mx = (float)rawMX;
    const float my = (float)rawMY;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float firstRowY = screenH * 0.35f; // move up/down to shift row hit zones
    const float rowStepY = 60.0f;           // gap between rows, must match DrawUI
    const float rowH = 34.0f;           // height of each click zone

    const float valueStartX = screenW * 0.48f; // where the value string starts, must match DrawUI
    const float charW_left = 22.0f;           // px per glyph used for the LEFT  < arrow position
    const float charW_right = 5.0f;           // px per glyph used for the RIGHT > arrow position (smaller = arrow moves right)
    const float arrowHalfW = 28.0f;           // half-width of each arrow's click zone (bigger = more forgiving)

    const float labelL = screenW * 0.18f;       // left edge of label (RESOLUTION / VOLUME) click zone
    const float labelR = screenW * 0.90f;       // right edge of label click zone

    // Back button
    const float backY = screenH * 0.72f;
    const float backH = 36.0f;
    const float backL = screenW * 0.30f;
    const float backR = screenW * 0.70f;

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (mx >= backL && mx <= backR && my >= backY && my <= backY + backH)
        {
            SceneManager::I().SwitchTo(SceneID::MainMenu);
            return;
        }
    }

    const int rowCount = (int)SettingRow::COUNT;

    for (int i = 0; i < rowCount; ++i)
    {
        float rowTop = firstRowY + i * rowStepY;
        float rowBot = rowTop + rowH;
        if (my < rowTop || my > rowBot) continue;

        // Build the same string DrawUI prints so we know the real length
        char valueBuf[64] = {};
        if (i == (int)SettingRow::Resolution)
        {
            sprintf_s(valueBuf, "< %s >",
                GameSettings::RESOLUTIONS[GameSettings::resolutionIndex].label);
        }
        else
        {
            const int barLen = 10;
            const int filled = (GameSettings::masterVolume * barLen) / 100;
            char bar[20] = "[";
            for (int b = 0; b < barLen; ++b)
                strcat_s(bar, b < filled ? "|" : " ");
            strcat_s(bar, "]");
            sprintf_s(valueBuf, "< %d%% %s >", GameSettings::masterVolume, bar);
        }

        int   len = (int)strlen(valueBuf);
        // < is the first character of the string
        float leftCX = valueStartX + charW_left * 0.5f;
        // > is the last character — uses charW_right so it lands further right
        float rightCX = valueStartX + (len - 0.5f) * charW_right;

        bool inLabel = (mx >= labelL && mx <= labelR);
        bool inLeft = (mx >= leftCX - arrowHalfW && mx <= leftCX + arrowHalfW);
        bool inRight = (mx >= rightCX - arrowHalfW && mx <= rightCX + arrowHalfW);

        // Hover: select row when cursor is over any interactive zone
        if (inLabel || inLeft || inRight)
            m_selected = (SettingRow)i;

        if (!AEInputCheckTriggered(AEVK_LBUTTON)) return;

        int delta = 0;
        if (inLeft)  delta = -1;
        else if (inRight) delta = 1;
        else if (inLabel) delta = 1; // label click cycles forward

        if (delta == 0) return;

        switch ((SettingRow)i)
        {
        case SettingRow::Resolution:
            GameSettings::resolutionIndex =
                (GameSettings::resolutionIndex + delta + GameSettings::RESOLUTION_COUNT)
                % GameSettings::RESOLUTION_COUNT;
            GameSettings::pendingRestart = true;
            GameSettings::Save();
            break;

        case SettingRow::Volume:
            GameSettings::masterVolume += delta * 5;
            if (GameSettings::masterVolume < 0)   GameSettings::masterVolume = 0;
            if (GameSettings::masterVolume > 100) GameSettings::masterVolume = 100;
            GameSettings::Save();
            ApplyVolume();
            break;

        default: break;
        }
        return;
    }
}

// DrawUI
void Scene_Settings::DrawUI() const
{
    if (m_uiFont < 0) return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float bright = 1.0f;
    const float dim = 0.35f;
    const float info = 0.70f;

    const float titleY = screenH * 0.18f;
    const float firstRowY = screenH * 0.38f;
    const float rowStepY = 60.0f;
    const float backY = screenH * 0.76f;
    const float noteY = backY + 34.0f;

    auto Print = [&](const char* text, float px, float py,
        float r, float g, float b, float scale = 1.0f)
        {
            AEGfxPrint(m_uiFont, text,
                ScreenToNormX(px), ScreenToNormY(py),
                scale, r, g, b, 1.0f);
        };

    // Title
    const char* title = "SETTINGS";
    Print(title, GetCenteredX(title, 1.5f), titleY, bright, bright, bright, 1.5f);

    // Rows
    const char* rowLabels[] = { "RESOLUTION", "VOLUME" };
    const int   rowCount = (int)SettingRow::COUNT;

    for (int i = 0; i < rowCount; ++i)
    {
        bool  sel = ((int)m_selected == i);
        float rowY = firstRowY + i * rowStepY;
        float shade = sel ? bright : dim;

        const float labelX = screenW * 0.22f;
        if (sel)
            Print(">", labelX - 28.0f, rowY, bright, bright, bright);

        Print(rowLabels[i], labelX, rowY, shade, shade, shade);

        char valueBuf[64] = {};
        if (i == (int)SettingRow::Resolution)
        {
            sprintf_s(valueBuf, "< %s >",
                GameSettings::RESOLUTIONS[GameSettings::resolutionIndex].label);
        }
        else
        {
            const int barLen = 10;
            const int filled = (GameSettings::masterVolume * barLen) / 100;
            char bar[20] = "[";
            for (int b = 0; b < barLen; ++b)
                strcat_s(bar, b < filled ? "|" : " ");
            strcat_s(bar, "]");
            sprintf_s(valueBuf, "< %d%% %s >", GameSettings::masterVolume, bar);
        }

        float vr = sel ? 0.8f : 0.45f;
        float vg = sel ? 0.85f : 0.55f;
        float vb = sel ? 1.0f : 0.70f;
        Print(valueBuf, screenW * 0.48f, rowY, vr, vg, vb);
    }

    // Controls hint
    const float hintY = firstRowY + rowCount * rowStepY + 20.0f;
    const char* hints[] = { "W / S       - Navigate", "A / LEFT    - Decrease", "D / RIGHT   - Increase" };
    for (int i = 0; i < 3; ++i)
        Print(hints[i], GetCenteredX(hints[i], 0.75f), hintY + i * 28.0f, info, info, info, 0.75f);

    // Back
    const char* backText = "BACK  (ESC)";
    Print(backText, GetCenteredX(backText, 1.0f), backY, bright, bright, bright);

    // Pending restart note
    if (GameSettings::pendingRestart)
    {
        const char* note = "* Resolution applies on next launch";
        Print(note, GetCenteredX(note, 0.7f), noteY, 1.0f, 0.8f, 0.2f, 0.7f);
    }
}

void Scene_Settings::Draw()
{
    AEGfxSetBackgroundColor(0.08f, 0.08f, 0.12f);
    DrawUI();
}