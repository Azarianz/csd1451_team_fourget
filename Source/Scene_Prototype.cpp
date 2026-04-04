// Scene_Prototype.cpp
#include "Scene_Prototype.h"
#include "SceneManager.h"
#include "GameSettings.h"
#include "AudioManager.h"

#include "AEEngine.h"
#include "AEInput.h"
#include "Utility.h"
#include <cstdio>
#include "GlobalFonts.h"

#pragma region Debug
void Scene_Prototype::HandleDebugInput()
{
    // F1 - Add gold
    if (AEInputCheckTriggered(AEVK_F1))
    {
        shop.AddPoints(1000);
        //PRINT("[DEBUG] +1000 GOLD\n");
    }

    // F2 - Kill all enemies
    if (AEInputCheckTriggered(AEVK_F2))
    {
        //PRINT("[DEBUG] CLEAR ALL ENEMIES\n");

        for (Enemy* e : enemies)
        {
            if (e)
                e->health = 0.0f;
        }
    }

    // F3 - Force win
    if (AEInputCheckTriggered(AEVK_F3))
    {
        //PRINT("[DEBUG] FORCE WIN\n");
        OpenWinPopup();
    }
}
#pragma endregion

#pragma region Helper Funcs
bool Scene_Prototype::IsBlockingPopupOpen() const
{
    return m_stageWon
        || gameOver
        || m_quitConfirmOpen
        || m_pauseMenuOpen;
}

#pragma region Quit Confirm Popup
void Scene_Prototype::OpenQuitConfirmPopup()
{
    m_quitConfirmOpen = true;
    m_paused = true;
}

void Scene_Prototype::CloseQuitConfirmPopup()
{
    m_quitConfirmOpen = false;
}

bool Scene_Prototype::IsInQuitYesButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 160.0f;
    const float buttonH = 24.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 145.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

bool Scene_Prototype::IsInQuitNoButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 120.0f;
    const float buttonH = 24.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 172.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

void Scene_Prototype::UpdateQuitConfirmPopup(int mouseX, int mouseY)
{
    if (!m_quitConfirmOpen)
        return;

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsInQuitYesButton(mouseX, mouseY))
        {
            AudioManager::ResumeBGM();
            SceneManager::I().SwitchTo(SceneID::MainMenu);
            return;
        }

        if (IsInQuitNoButton(mouseX, mouseY))
        {
            CloseQuitConfirmPopup();
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
    {
        AudioManager::ResumeBGM();
        SceneManager::I().SwitchTo(SceneID::MainMenu);
        return;
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        CloseQuitConfirmPopup();
        return;
    }
}

void Scene_Prototype::DrawQuitConfirmPopup() const
{
    if (!m_quitConfirmOpen)
        return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const bool hoverYes = IsInQuitYesButton(mouseX, mouseY);
    const bool hoverNo = IsInQuitNoButton(mouseX, mouseY);

    auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
    auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

    auto GetCenteredX = [](const char* text, float scale, float areaCenterX)
        {
            const float charWidthPx = 22.0f * scale;
            const float textWidth = (float)std::strlen(text) * charWidthPx;
            return areaCenterX - textWidth * 0.5f;
        };

    const float msgY = top + 100.0f;
    const float yesY = top + 155.0f;
    const float noY = top + 195.0f;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(0.95f);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
        -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxVertexList* overlayMesh = AEGfxMeshEnd();

    if (overlayMesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, 0.0f, 0.0f);
        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(overlayMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(overlayMesh);
    }

    if (m_uiFont >= 0)
    {
        const char* titleText = "CONFIRM";
        const char* msgText = "ARE YOU SURE YOU WANT TO LEAVE THE GAME?";
        const char* yesText = "YES";
        const char* noText = "NO";

        const float bright = 1.0f;
        const float dim = 0.35f;

        const float titleScale = 0.75f;
        const float optionScale = 1.0f;

        const float titleX = GetCenteredX(titleText, titleScale, centerX - 100.0f) + 90.f;
        const float titleY = top + 64.0f;

        const float msgTextX = GetCenteredX(msgText, optionScale, centerX) - 100.f;
        const float yesTextX = GetCenteredX(yesText, optionScale, centerX);
        const float noTextX = GetCenteredX(noText, optionScale, centerX);

        AEGfxPrint(
            gameOverFont >= 0 ? gameOverFont : m_uiFont,
            titleText,
            ToNdcX(titleX),
            ToNdcY(titleY),
            titleScale,
            1.0f, 1.0f, 0.2f, 1.0f
        );

        AEGfxPrint(
            m_uiFont,
            msgText,
            ToNdcX(msgTextX),
            ToNdcY(msgY),
            optionScale,
            1.0f, 1.0f, 1.0f, 1.0f
        );

        AEGfxPrint(
            m_uiFont,
            yesText,
            ToNdcX(yesTextX),
            ToNdcY(yesY),
            optionScale,
            hoverYes ? bright : dim,
            hoverYes ? bright : dim,
            hoverYes ? bright : dim,
            1.0f
        );

        AEGfxPrint(
            m_uiFont,
            noText,
            ToNdcX(noTextX),
            ToNdcY(noY),
            optionScale,
            hoverNo ? bright : dim,
            hoverNo ? bright : dim,
            hoverNo ? bright : dim,
            1.0f
        );
    }
}
#pragma endregion

#pragma region Win Popup
void Scene_Prototype::OpenWinPopup()
{
    m_stageWon = true;
    m_paused = true;
    AudioManager::PauseBGM();
}

bool Scene_Prototype::IsInNextStageButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 190.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 86.0f; // match draw row

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

bool Scene_Prototype::IsInMainMenuButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 180.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 140.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

void Scene_Prototype::UpdateWinPopup(int mouseX, int mouseY)
{
    if (!m_stageWon)
        return;

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsInNextStageButton(mouseX, mouseY))
        {
            if (!LoadNextLevel())
            {
                //PRINT("No next stage configured. Returning to main menu.\n");
                AudioManager::ResumeBGM();
                SceneManager::I().SwitchTo(SceneID::MainMenu);
            }
            return;
        }

        if (IsInMainMenuButton(mouseX, mouseY))
        {
            AudioManager::ResumeBGM();
            SceneManager::I().SwitchTo(SceneID::MainMenu);
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
    {
        if (!LoadNextLevel())
        {
            //PRINT("No next stage configured. Returning to main menu.\n");
            AudioManager::ResumeBGM();
            SceneManager::I().SwitchTo(SceneID::MainMenu);
        }
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        AudioManager::ResumeBGM();
        SceneManager::I().SwitchTo(SceneID::MainMenu);
    }
}

void Scene_Prototype::DrawWinPopup() const
{
    if (!m_stageWon)
        return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const bool hoverNext = IsInNextStageButton(mouseX, mouseY);
    const bool hoverMenu = IsInMainMenuButton(mouseX, mouseY);

    auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
    auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

    auto GetCenteredX = [](const char* text, float scale, float areaCenterX)
        {
            const float charWidthPx = 22.0f * scale;
            const float textWidth = (float)std::strlen(text) * charWidthPx;
            return areaCenterX - textWidth * 0.5f;
        };

    // same-sized hit/button areas used by hover + click
    const float nextY = top + 118.0f;
    const float menuY = top + 172.0f;

    // dark fullscreen overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(0.95f);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f, -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxVertexList* overlayMesh = AEGfxMeshEnd();

    if (overlayMesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, 0.0f, 0.0f);
        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(overlayMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(overlayMesh);
    }

    if (m_uiFont >= 0)
    {
        const char* titleText = "YOU WIN";
        const char* nextText = "NEXT STAGE";
        const char* menuText = "MAIN MENU";

        const float bright = 1.0f;
        const float dim = 0.35f;

        const float titleScale = 1.25f;
        const float optionScale = 1.0f;

        const float titleX = GetCenteredX(titleText, titleScale, centerX - 180) + 180.f;
        const float titleY = top + 64.0f;

        const float nextTextX = GetCenteredX(nextText, optionScale, centerX);
        const float nextTextY = nextY + 6.0f;

        const float menuTextX = GetCenteredX(menuText, optionScale, centerX);
        const float menuTextY = menuY + 6.0f;

        AEGfxPrint(
            gameOverFont >= 0 ? gameOverFont : m_uiFont,
            titleText,
            ToNdcX(titleX),
            ToNdcY(titleY),
            titleScale,
            0.2f, 1.0f, 0.3f, 1.0f
        );

        AEGfxPrint(
            m_uiFont,
            nextText,
            ToNdcX(nextTextX),
            ToNdcY(nextTextY),
            optionScale,
            hoverNext ? bright : dim,
            hoverNext ? bright : dim,
            hoverNext ? bright : dim,
            1.0f
        );

        AEGfxPrint(
            m_uiFont,
            menuText,
            ToNdcX(menuTextX),
            ToNdcY(menuTextY),
            optionScale,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            1.0f
        );
    }
}
#pragma endregion

#pragma region Lose Popup
void Scene_Prototype::OpenLosePopup()
{
    gameOver = true;
    m_paused = true;
    AudioManager::PauseBGM();
}

bool Scene_Prototype::IsInRetryButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 190.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 86.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

void Scene_Prototype::UpdateLosePopup(int mouseX, int mouseY)
{
    if (!gameOver)
        return;

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsInRetryButton(mouseX, mouseY))
        {
            AudioManager::ResumeBGM();
            SceneManager::I().SwitchTo(SceneID::Prototype);
            return;
        }

        if (IsInMainMenuButton(mouseX, mouseY))
        {
            AudioManager::ResumeBGM();
            SceneManager::I().SwitchTo(SceneID::MainMenu);
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
    {
        AudioManager::ResumeBGM();
        SceneManager::I().SwitchTo(SceneID::Prototype);
        return;
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        AudioManager::ResumeBGM();
        SceneManager::I().SwitchTo(SceneID::MainMenu);
    }
}

void Scene_Prototype::DrawLosePopup() const
{
    if (!gameOver)
        return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const bool hoverRetry = IsInRetryButton(mouseX, mouseY);
    const bool hoverMenu = IsInMainMenuButton(mouseX, mouseY);

    auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
    auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

    auto GetCenteredX = [](const char* text, float scale, float areaCenterX)
        {
            const float charWidthPx = 22.0f * scale;
            const float textWidth = (float)std::strlen(text) * charWidthPx;
            return areaCenterX - textWidth * 0.5f;
        };

    const float nextY = top + 118.0f;
    const float menuY = top + 172.0f;

    // dark fullscreen overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(0.95f);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f, 0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f, -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxVertexList* overlayMesh = AEGfxMeshEnd();

    if (overlayMesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, 0.0f, 0.0f);
        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(overlayMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(overlayMesh);
    }

    if (m_uiFont >= 0)
    {
        const char* titleText = "GAME OVER";
        const char* retryText = "RETRY";
        const char* menuText = "MAIN MENU";

        const float bright = 1.0f;
        const float dim = 0.35f;

        const float titleScale = 1.25f;
        const float optionScale = 1.0f;

        const float titleX = GetCenteredX(titleText, titleScale, centerX - 210) + 200.f;
        const float titleY = top + 64.0f;

        const float retryTextX = GetCenteredX(retryText, optionScale, centerX);
        const float retryTextY = nextY + 6.0f;

        const float menuTextX = GetCenteredX(menuText, optionScale, centerX);
        const float menuTextY = menuY + 6.0f;

        AEGfxPrint(
            gameOverFont >= 0 ? gameOverFont : m_uiFont,
            titleText,
            ToNdcX(titleX),
            ToNdcY(titleY),
            titleScale,
            1.0f, 0.1f, 0.1f, 1.0f
        );

        AEGfxPrint(
            m_uiFont,
            retryText,
            ToNdcX(retryTextX),
            ToNdcY(retryTextY),
            optionScale,
            hoverRetry ? bright : dim,
            hoverRetry ? bright : dim,
            hoverRetry ? bright : dim,
            1.0f
        );

        AEGfxPrint(
            m_uiFont,
            menuText,
            ToNdcX(menuTextX),
            ToNdcY(menuTextY),
            optionScale,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            1.0f
        );
    }
}
#pragma endregion

#pragma region Pause Popup
void Scene_Prototype::OpenPausePopup()
{
    m_pauseMenuOpen = true;
    m_paused = true;

    if (m_tutorialPopup.IsActive())
        m_tutorialPopup.Close();

    m_showCodex = false;

    AudioManager::PauseBGM();
}

void Scene_Prototype::ClosePausePopup()
{
    m_pauseMenuOpen = false;
    m_paused = false;
    AudioManager::ResumeBGM();
}

bool Scene_Prototype::IsInResumeButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 190.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 86.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

bool Scene_Prototype::IsInGuideButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 190.0f;
    const float buttonH = 22.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 114.f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

void Scene_Prototype::UpdatePausePopup(int mouseX, int mouseY)
{
    if (!m_pauseMenuOpen)
        return;

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsInResumeButton(mouseX, mouseY))
        {
            ClosePausePopup();
            return;
        }

        if (IsInGuideButton(mouseX, mouseY))
        {
            ClosePausePopup();
            m_tutorialPopup.ForceStart();
            return;
        }

        if (IsInMainMenuButton(mouseX, mouseY))
        {
            OpenQuitConfirmPopup();
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_P))
    {
        ClosePausePopup();
        return;
    }
}

void Scene_Prototype::DrawPausePopup() const
{
    if (!m_pauseMenuOpen)
        return;

    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const bool hoverResume = IsInResumeButton(mouseX, mouseY);
    const bool hoverGuide = IsInGuideButton(mouseX, mouseY);
    const bool hoverMenu = IsInMainMenuButton(mouseX, mouseY);

    auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
    auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

    auto GetCenteredX = [](const char* text, float scale, float areaCenterX)
        {
            const float charWidthPx = 22.0f * scale;
            const float textWidth = (float)std::strlen(text) * charWidthPx;
            return areaCenterX - textWidth * 0.5f;
        };

    const float resumeY = top + 118.0f;
    const float guideY = top + 145.0f;
    const float menuY = top + 172.0f;

    // dark fullscreen overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(0.95f);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
        0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
        -0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);
    AEGfxVertexList* overlayMesh = AEGfxMeshEnd();

    if (overlayMesh)
    {
        AEMtx33 scaleM, rotM, transM, finalMtx;
        AEMtx33Scale(&scaleM, screenW, screenH);
        AEMtx33Rot(&rotM, 0.0f);
        AEMtx33Trans(&transM, 0.0f, 0.0f);
        AEMtx33Concat(&finalMtx, &rotM, &scaleM);
        AEMtx33Concat(&finalMtx, &transM, &finalMtx);
        AEGfxSetTransform(finalMtx.m);
        AEGfxMeshDraw(overlayMesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(overlayMesh);
    }

    if (m_uiFont >= 0)
    {
        const char* titleText = "PAUSED";
        const char* resumeText = "RESUME";
        const char* guideText = "GUIDE";
        const char* menuText = "MAIN MENU";

        const float bright = 1.0f;
        const float dim = 0.35f;

        const float titleScale = 1.25f;
        const float optionScale = 1.0f;

        const float titleX = GetCenteredX(titleText, titleScale, centerX - 130.0f) + 130.f;
        const float titleY = top + 64.0f;

        const float resumeTextX = GetCenteredX(resumeText, optionScale, centerX);
        const float resumeTextY = resumeY + 6.0f;

        const float guideTextX = GetCenteredX(guideText, optionScale, centerX);
        const float guideTextY = guideY + 6.0f;

        const float menuTextX = GetCenteredX(menuText, optionScale, centerX);
        const float menuTextY = menuY + 6.0f;

        AEGfxPrint(
            gameOverFont >= 0 ? gameOverFont : m_uiFont,
            titleText,
            ToNdcX(titleX),
            ToNdcY(titleY),
            titleScale,
            1.0f, 1.0f, 0.2f, 1.0f
        );

        AEGfxPrint(
            m_uiFont,
            resumeText,
            ToNdcX(resumeTextX),
            ToNdcY(resumeTextY),
            optionScale,
            hoverResume ? bright : dim,
            hoverResume ? bright : dim,
            hoverResume ? bright : dim,
            1.0f
        );

        AEGfxPrint(
            m_uiFont,
            guideText,
            ToNdcX(guideTextX),
            ToNdcY(guideTextY),
            optionScale,
            hoverGuide ? bright : dim,
            hoverGuide ? bright : dim,
            hoverGuide ? bright : dim,
            1.0f
        );

        AEGfxPrint(
            m_uiFont,
            menuText,
            ToNdcX(menuTextX),
            ToNdcY(menuTextY),
            optionScale,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            hoverMenu ? bright : dim,
            1.0f
        );
    }
}
#pragma endregion

#pragma region Tutorial Helpers
bool Scene_Prototype::IsTutorialLevel() const
{
    // Adjust these names to match your actual tutorial file names
    return (levelFile.find("level_01") != std::string::npos);
}

void Scene_Prototype::UpdateTutorialPopup()
{
    bool wasActive = m_tutorialPopup.IsActive();

    // Do not allow tutorial to open while any hard popup or codex is open
    if (!IsBlockingPopupOpen() && !m_showCodex)
    {
        if (AEInputCheckTriggered(AEVK_T) && !m_tutorialPopup.IsActive())
        {
            m_tutorialPopup.ForceStart();
        }

        if (IsTutorialLevel() && !m_tutorialPopup.HasStarted())
        {
            m_tutorialPopup.Start();
        }
    }

    m_tutorialPopup.Update();

    bool isActive = m_tutorialPopup.IsActive();

    if (!wasActive && isActive)
    {
        m_paused = true;
    }
    else if (wasActive && !isActive)
    {
        m_paused = false;
    }
}
#pragma endregion

#pragma region Codex
void Scene_Prototype::UpdateCodex()
{
    // Do not allow codex to open while any hard popup or tutorial is open
    if (!IsBlockingPopupOpen() && !m_tutorialPopup.IsActive())
    {
        if (AEInputCheckTriggered(AEVK_TAB))
        {
            m_showCodex = !m_showCodex;

            m_paused = m_showCodex;
        }
    }

    if (m_showCodex && AEInputCheckTriggered(AEVK_ESCAPE))
    {
        m_showCodex = false;
        m_paused = false;
    }
}

void Scene_Prototype::DrawCodex() const
{
    if (!m_showCodex || !m_codexTex)
        return;

    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();

    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(m_codexTex, 0, 0);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    AEMtx33 scaleM, rotM, transM, finalMtx;

    AEMtx33Scale(&scaleM, screenW, screenH);
    AEMtx33Rot(&rotM, 0.0f);
    AEMtx33Trans(&transM, 0.0f, 0.0f);

    AEMtx33Concat(&finalMtx, &rotM, &scaleM);
    AEMtx33Concat(&finalMtx, &transM, &finalMtx);

    AEGfxSetTransform(finalMtx.m);

    // Fullscreen quad
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
        AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
        AEGfxMeshFree(mesh);
    }
}
#pragma endregion

bool Scene_Prototype::IsStageCleared() const
{
    return waveManager.waveComplete && enemies.empty();
}

bool Scene_Prototype::LoadNextLevel() const
{
    std::string nextLevel = levelFile;

    if (nextLevel.find("level_01.txt") != std::string::npos)
    {
        nextLevel.replace(nextLevel.find("level_01.txt"), std::string("level_01.txt").length(), "level_02.txt");
    }
    else if (nextLevel.find("level_02.txt") != std::string::npos)
    {
        nextLevel.replace(nextLevel.find("level_02.txt"), std::string("level_02.txt").length(), "level_03.txt");
    }
    else if (nextLevel.find("level_03.txt") != std::string::npos)
    {
        //PRINT("Final stage cleared. Going to credits.\n");
        SceneManager::I().SwitchTo(SceneID::Credits);
        return true;
    }
    else
    {
        return false; // no next level configured
    }

    GameSettings::selectedLevelFile = nextLevel;
    //PRINT("Loading next level file: %s\n", GameSettings::selectedLevelFile.c_str());

    AudioManager::ResumeBGM();
    SceneManager::I().SwitchTo(SceneID::Prototype);
    return true;
}
// --------------------------------------------------------
//  Tutorial Helpers
// --------------------------------------------------------

static AEGfxVertexList* CreateFlagMesh(int row, int col)
{
    int SHEET_COLS = 18;
    int SHEET_ROWS = 11;
    int ty = (SHEET_ROWS - 1) - row;
    float u0 = (float)col / (float)SHEET_COLS;
    float u1 = (float)(col + 1) / (float)SHEET_COLS;
    float v0 = (float)ty / (float)SHEET_ROWS;
    float v1 = (float)(ty + 1) / (float)SHEET_ROWS;

    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, u0, v1, 0.5f, -0.5f, 0xFFFFFFFF, u1, v1, 0.5f, 0.5f, 0xFFFFFFFF, u1, v0);
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, u0, v1, 0.5f, 0.5f, 0xFFFFFFFF, u1, v0, -0.5f, 0.5f, 0xFFFFFFFF, u0, v0);
    return AEGfxMeshEnd();
}

// --------------------------------------------------------
//  DestroyGrid
//  Safely destroys the current grid instance and clears pointer.
// --------------------------------------------------------
void Scene_Prototype::DestroyGrid()
{
    if (grid)
    {
        grid->Destroy();
        delete grid;
        grid = nullptr;
    }
}

// --------------------------------------------------------
//  InitLevelAndGrid
//  Loads the level data, creates the grid system, and builds
//  the enemy path used for navigation.
// --------------------------------------------------------
bool Scene_Prototype::InitLevelAndGrid()
{
    GameSettings::levelLoadFailed = false;
    GameSettings::levelLoadError = "";

    //Load Level File
    levelFile = GameSettings::selectedLevelFile;

    //PRINT("Prototype received level file: %s\n", levelFile.c_str());

    if (levelFile.empty())
    {
        PRINT("Level file empty, defaulting to Assets/Levels/level_01.txt\n");
        levelFile = "Assets/Levels/level_01.txt";
    }

    //PRINT("Loading level file: %s\n", levelFile.c_str());

    if (!level.Init(levelFile.c_str()))
    {
        //("FAILED to load level: %s\n", levelFile.c_str());
        GameSettings::levelLoadFailed = true;
        GameSettings::levelLoadError = "Failed to load level file.";
        return false;
    }

    occupied.assign((size_t)level.width * (size_t)level.height, 0);

    grid = new GridSystem::Grid(level.width, level.height, 1.0f);
    grid->Init();

    path.clear();
    if (!level.BuildPath(*grid, path))
    {
        //("FAILED to build enemy path from region flags.\n");
        GameSettings::levelLoadFailed = true;
        GameSettings::levelLoadError = "Invalid enemy path, spawn, or goal.";
        return false;
    }

    return true;
}

// --------------------------------------------------------
//  DrawUI
//  Renders gameplay UI information.
//  - Displays current wave progress
//  - Displays active enemy count
//  - Draws pause/resume label in the top-right
// --------------------------------------------------------
void Scene_Prototype::DrawUI() const
{
    if (m_uiFont < 0) return;

    char buf[64];

    // --- 1. WAVE COUNTER ---
    if (waveManager.waveComplete) {
        sprintf_s(buf, "WAVES COMPLETE!");
    }
    else {
        sprintf_s(buf, "WAVE: %d / %d", waveManager.GetWavestarterCount(), waveManager.GetTotalWavestarters());
    }
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.90f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 2. ENEMY COUNTER ---
    int enemiesRemaining = (int)enemies.size();

    if (!waveManager.waveComplete) {
        int unspawnedEnemies = waveManager.GetTotalEnemiesInCurrentWave() - waveManager.spawnedInCurrentWave;
        enemiesRemaining += unspawnedEnemies;
    }

    sprintf_s(buf, "ENEMIES: %d", enemiesRemaining);
    AEGfxPrint(m_uiFont, buf, -0.95f, 0.80f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    // --- 3. WAVE TIMER ---
    if (!waveManager.waveComplete && waveManager.IsWaitingForWavestarter()) {
        sprintf_s(buf, "NEXT WAVE IN: %.1f", waveManager.GetTimeUntilNextSpawn());
        AEGfxPrint(m_uiFont, buf, -0.2f, 0.90f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    }

    // Get screen dimensions for dynamic positioning
    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();

    // --- 4. GAME SPEED INDICATOR (FLAG) ---
    sprintf_s(buf, "SPEED: ");

    // moved up to where PAUSE used to be
    float speedTextNdcX = 0.67f;
    float speedTextNdcY = 0.82f;
    AEGfxPrint(m_uiFont, buf, speedTextNdcX, speedTextNdcY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

    int flagIndex = 0; // White Flag (1.0x)
    if (gameSpeedMultiplier == 2.0f) flagIndex = 1; // Green Flag (2.0x)
    else if (gameSpeedMultiplier == 4.0f) flagIndex = 2; // Blue Flag (4.0x)

    if (m_spriteSheet && m_flagMeshes[flagIndex])
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(m_spriteSheet, 0, 0);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetTransparency(1.0f);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

        AEMtx33 scaleM, rotM, transM, transform;
        AEMtx33Scale(&scaleM, 40.0f, 40.0f);
        AEMtx33Rot(&rotM, 0.0f);

        float flagWorldX = (screenW * 0.5f) * 0.92f;
        float flagWorldY = (screenH * 0.5f) * 0.84f;
        AEMtx33Trans(&transM, flagWorldX, flagWorldY);

        AEMtx33Concat(&transform, &rotM, &scaleM);
        AEMtx33Concat(&transform, &transM, &transform);

        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(m_flagMeshes[flagIndex], AE_GFX_MDM_TRIANGLES);

        //Reset
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetTransparency(1.0f);
    }
}
// --------------------------------------------------------
//  IsPauseButtonClicked
//  Checks whether the mouse click is inside the pause button.
//  - Calculates the UI text position
//  - Builds a hitbox around the pause label
//  - Returns true if the cursor is inside the hit area
// --------------------------------------------------------
bool Scene_Prototype::IsPauseButtonClicked(int mouseX, int mouseY) const
{
    float screenW = (float)AEGfxGetWindowWidth();
    float screenH = (float)AEGfxGetWindowHeight();

    float textX = screenW * 0.88f;
    float textY = screenH * 0.08f;

    float hitW = screenW * 0.10f;
    float hitH = screenH * 0.06f;

    float top = textY - hitH;
    float bottom = textY;

    return ((float)mouseX >= textX && (float)mouseX <= textX + hitW &&
        (float)mouseY >= top && (float)mouseY <= bottom);
}

// --------------------------------------------------------
//  ResetRuntimeState
//  Clears all gameplay containers and resets runtime flags.
//  - Clears towers, bullets and enemies
//  - Resets mouse state
//  - Clears game over and pause flags
// --------------------------------------------------------
void Scene_Prototype::ResetRuntimeState()
{
    for (Enemy* e : enemies)
        delete e;
    enemies.clear();

    activeTowers.clear();
    activeBullets.clear();

    wasLmbDown = false;
    baseTowerIndex = -1;
    gameOver = false;
    m_stageWon = false;
    m_pauseMenuOpen = false;
    m_paused = false;
    gameSpeedMultiplier = 1.0f;

    m_quitConfirmOpen = false;
}

// --------------------------------------------------------
//  HandleUserInputs
//  Processes player input for pause, debug upgrade, and
//  tower selection / dragging interactions.
// --------------------------------------------------------
void Scene_Prototype::HandleUserInputs(float worldX, float worldY, int mouseX, int mouseY)
{
    //Pause Toggle
    bool openPause = false;

    if (AEInputCheckTriggered(AEVK_ESCAPE) || AEInputCheckTriggered(AEVK_P))
        openPause = true;

    if (AEInputCheckTriggered(AEVK_LBUTTON) && IsPauseButtonClicked(mouseX, mouseY))
        openPause = true;

    if (openPause)
    {
        if (m_pauseMenuOpen)
            ClosePausePopup();
        else
            OpenPausePopup();
    }

    if (m_paused)
        return;

    if (AEInputCheckTriggered(AEVK_SPACE))
    {
        if (gameSpeedMultiplier == 1.0f) gameSpeedMultiplier = 2.0f;
        else if (gameSpeedMultiplier == 2.0f) gameSpeedMultiplier = 4.0f;
        else gameSpeedMultiplier = 1.0f;
    }

    // Debug Upgrade tower
    if (AEInputCheckTriggered(AEVK_U))
    {
        for (auto& t : activeTowers)
        {
            if (t.isSelected && !t.isDragging)
            {
                t.LevelUp();
                break;
            }
        }
    }

    UpdateSelectionAndDragging(worldX, worldY, mouseX, mouseY);
}

// --------------------------------------------------------
//  UpdateSelectionAndDragging
//  Handles mouse interactions with towers.
//  - Updates shop UI interactions
//  - Processes tower dragging and placement    
//  - Selects the topmost tower on click
// --------------------------------------------------------
void Scene_Prototype::UpdateSelectionAndDragging(float worldX, float worldY, int mouseX, int mouseY)
{
    bool lmbDown = AEInputCheckCurr(AEVK_LBUTTON);
    bool justPressedLmb = (!wasLmbDown && lmbDown);
    bool justReleasedLmb = (wasLmbDown && !lmbDown);

    shop.Update(activeTowers);

    //Retrofitted code now inside here
    buildMergeSystem.UpdateDragging(worldX, worldY, lmbDown, justReleasedLmb, mouseX, mouseY);

    if (justPressedLmb && !buildMergeSystem.IsDraggingTower())
        TowerHandler::SelectTopmostTower(worldX, worldY, activeTowers);

    wasLmbDown = lmbDown;
}

// --------------------------------------------------------
//  UpdateEnemies
//  Handles enemy spawning from the wave manager and updates
//  all active enemies along the path.
// --------------------------------------------------------
void Scene_Prototype::UpdateEnemies(float dt)
{
    Enemy* spawnedEnemy = waveManager.UpdateAndSpawn(dt, path);
    if (spawnedEnemy)
        enemies.push_back(spawnedEnemy);

    for (Enemy* e : enemies)
    {
        if (e)
            e->Update(dt, path);
    }
}

// --------------------------------------------------------
//  UpdateBaseCollision
//  Handles collisions between enemies and the base tower.
//  - Enemies reaching the base are marked dead/escaped
//  - Base takes contact damage
//  - Triggers game over if base health reaches zero
// --------------------------------------------------------
void Scene_Prototype::UpdateBaseCollision()
{
    if (baseTowerIndex < 0 || baseTowerIndex >= (int)activeTowers.size())
        return;

    TowerHandler::Tower& base = activeTowers[(size_t)baseTowerIndex];

    for (Enemy* e : enemies)
    {
        if (!e || e->health <= 0.0f)
            continue;

        if (TowerHandler::CircleCircleCollision(
            base.x, base.y, base._sizeX * 0.5f,
            e->x, e->y, e->_sizeX * 0.5f) ||
            e->reachedEnd)
        {
            e->health = 0.0f;
            e->escapedBase = true;
            AudioManager::PlaySFX("Assets/Audio/BaseHit.wav", GameSettings::masterVolume / 100.0f);

			// Null any bullet targets pointing to this enemy to prevent dangling pointers
            for (auto& b : activeBullets)
            {
                if (b.target == e) b.target = nullptr;
            }

            if (base.TakeDamage(base.details.contactDamage))
            {
                OpenLosePopup();
            }
        }
    }
}

// --------------------------------------------------------
//  UpdateReturningTowers
//  Handles towers being returned to the shop.
//  - Moves towers toward their return target
//  - Refunds cost when they reach the shop slot
//  - Removes the tower from the active list
// --------------------------------------------------------
void Scene_Prototype::UpdateReturningTowers(float dt)
{
    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
    {
        TowerHandler::Tower& t = activeTowers[(size_t)i];
        if (!t.isReturning)
            continue;

        const float lerpSpeed = 8.0f;
        t.x += (t.returnTargetX - t.x) * lerpSpeed * dt;
        t.y += (t.returnTargetY - t.y) * lerpSpeed * dt;

        float dx = t.returnTargetX - t.x;
        float dy = t.returnTargetY - t.y;
        if (dx * dx + dy * dy < 4.0f)
        {
            shop.RefundTower(t.sourceSlotIndex, t.purchaseCost);
            buildMergeSystem.RemoveTowerAtIndex(i);
            buildMergeSystem.RebuildOccupiedFromTowers();
        }
    }
}

// --------------------------------------------------------
//  CleanupDeadEnemies
//  Removes defeated enemies from the scene.
//  - Awards points for enemies killed normally
//  - Does not reward enemies that escaped to the base
// --------------------------------------------------------
void Scene_Prototype::CleanupDeadEnemies()
{
    for (int i = (int)enemies.size() - 1; i >= 0; --i)
    {
        Enemy* e = enemies[(size_t)i];
        if (!e || e->health <= 0.0f)
        {
            if (e && !e->escapedBase)
            {
                // Award points
                shop.AddPoints(e->GetPoints());

                // TRIGGER DEATH PARTICLES HERE
                ParticleSystem::SpawnBurst(e->x, e->y,
                                            0.6f, 0.6f, 0.6f, // Grey color
                                            ParticleSystem::BurstSize::LARGE);
            }

			// Nulls bullet targets pointing to an enemy before 
            // deleting it to prevent dangling pointers (FIXED TYPO HERE)
            for(auto& b : activeBullets)
            {
				if (b.target == e) b.target = nullptr;
            }

            delete e;
            enemies.erase(enemies.begin() + i);
        }
    }
}
// --------------------------------------------------------
//  CreateBaseTower
//  Spawns the base tower at the end of the enemy path.
// --------------------------------------------------------
void Scene_Prototype::CreateBaseTower()
{
    if (!path.empty())
    {
        baseTowerIndex = TowerHandler::AddBaseTower(
            activeTowers,
            path.back().x,
            path.back().y,
            TowerHandler::g_BaseTowerStats.sizeX,
            TowerHandler::g_BaseTowerStats.sizeY
        );

        if (baseTowerIndex >= 0 && baseTowerIndex < (int)activeTowers.size())
            activeTowers[(size_t)baseTowerIndex].isPlaced = true;

        buildMergeSystem.RebuildOccupiedFromTowers();
    }
}
#pragma endregion

#pragma region Scene Funcs
void Scene_Prototype::Init()
{
    DestroyGrid();

    ResetRuntimeState();

    if (!InitLevelAndGrid())
    {
        //("Scene_Prototype Init failed to load level.\n");

        if (gameOverFont < 0)
			gameOverFont = g_TitleFont28;

        if (m_uiFont < 0)
            m_uiFont = g_TitleFont28;

        return;
    }

    TowerHandler::LoadTowerAssets();

    shop.Init();
    buildMergeSystem.Init(&level, grid, &shop, &activeTowers, &activeBullets, &occupied);

    CreateBaseTower();

    if (gameOverFont < 0)
        gameOverFont = g_TitleFont28;

    if (m_uiFont < 0)
        m_uiFont = g_TitleFont28;

    // Load level easy/ Hard
    std::string baseName = "level_01";
    size_t lastSlash = levelFile.find_last_of("/\\");
    size_t lastDot = levelFile.find_last_of(".");
    if (lastSlash != std::string::npos && lastDot != std::string::npos) {
        baseName = levelFile.substr(lastSlash + 1, lastDot - lastSlash - 1);
    }

    std::string waveFilePath = "Assets/Waves/" + baseName;
    if (GameSettings::currentDifficulty == GameSettings::Difficulty::Easy) {
        waveFilePath += "_easy.txt";
    }
    else {
        waveFilePath += "_hard.txt";
    }

    if (!waveManager.LoadFromFile(waveFilePath))
    {
        //PRINT("Failed to load %s! Falling back to Assets/waves.txt\n", waveFilePath.c_str());

        if (!waveManager.LoadFromFile("Assets/waves.txt")) {
            //PRINT("Failed to load fallback waves.txt!\n");
        }
    }

    //Codex assets
    m_codexTex = AEGfxTextureLoad("Assets/Tutorial/codex.png");

	//Tutorial assets
    m_spriteSheet = AEGfxTextureLoad("Assets/Sprites/rawspritesheet.png");
    m_flagMeshes[0] = CreateFlagMesh(9, 17);
    m_flagMeshes[1] = CreateFlagMesh(6, 17);
    m_flagMeshes[2] = CreateFlagMesh(7, 17);

    m_tutorialPopup.Init();
    m_tutorialPopup.Reset();

    m_tutorialPopup.SetEnabled(IsTutorialLevel());
    //PRINT("IsTutorialLevel: %d\n", IsTutorialLevel());
    m_tutorialPopup.SetSlides({
        "Assets/Tutorial/controls.png",
        "Assets/Tutorial/tutorial_01.png",
        "Assets/Tutorial/tutorial_02.png",
        "Assets/Tutorial/tutorial_03.png",
        "Assets/Tutorial/tutorial_04.png",
        "Assets/Tutorial/tutorial_05.png",
        "Assets/Tutorial/tutorial_06.png",
        "Assets/Tutorial/codex.png"
        });
}

void Scene_Prototype::Update(float dt)
{
    const float MAX_DT = 1.0f / 60.0f;
    if (dt > MAX_DT) dt = MAX_DT;

    if (GameSettings::levelLoadFailed)
    {
        if (AEInputCheckTriggered(AEVK_ESCAPE))
        {
            SceneManager::I().SwitchTo(SceneID::LevelSelect);
        }
        return;
    }

    if (!grid) return;

    int mouseX = 0, mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    if (m_stageWon)
    {
        UpdateWinPopup(mouseX, mouseY);
        return;
    }
    else if (gameOver)
    {
        UpdateLosePopup(mouseX, mouseY);
        return;
    }
    else if (m_quitConfirmOpen)
    {
        UpdateQuitConfirmPopup(mouseX, mouseY);
        return;
    }
    else if (m_pauseMenuOpen)
    {
        UpdatePausePopup(mouseX, mouseY);
        return;
    }

    // tutorial / codex only when no higher-priority popup is open
    bool tutorialWasActive = m_tutorialPopup.IsActive();
    UpdateTutorialPopup();
    if (tutorialWasActive || m_tutorialPopup.IsActive())
        return;

    UpdateCodex();
    if (m_showCodex)
        return;

    // DEBUG INPUTS
    HandleDebugInput();

    float worldX = 0.0f, worldY = 0.0f;
    Utility::GetWorldMousePos(worldX, worldY);

    HandleUserInputs(worldX, worldY, mouseX, mouseY);
    if (m_paused)
        return;

    float scaled_dt = dt * gameSpeedMultiplier;

    UpdateEnemies(scaled_dt);
    UpdateBaseCollision();
    CleanupDeadEnemies();
    TowerHandler::UpdateTowerLogic(scaled_dt, activeTowers, enemies, activeBullets);
    UpdateReturningTowers(scaled_dt);
    ParticleSystem::Update(scaled_dt);

    if (IsStageCleared() && !m_stageWon)
    {
        OpenWinPopup();
        UpdateWinPopup(mouseX, mouseY);
        return;
    }
}

void Scene_Prototype::Draw()
{
    if (GameSettings::levelLoadFailed)
    {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

        if (m_uiFont < 0)
            return;

        const float screenW = (float)AEGfxGetWindowWidth();
        const float screenH = (float)AEGfxGetWindowHeight();

        auto PrintCentered = [&](const char* text, float py, float scale,
            float r = 1.0f, float g = 1.0f, float b = 1.0f)
            {
                float px = screenW * 0.5f - (float)std::strlen(text) * 22.0f * scale * 0.5f;
                float nx = (px / screenW) * 2.0f - 1.0f;
                float ny = 1.0f - (py / screenH) * 2.0f;

                AEGfxPrint(m_uiFont, text, nx, ny, scale, r, g, b, 1.0f);
            };

        PrintCentered("FAILED TO LOAD LEVEL", screenH * 0.40f, 1.1f, 1.0f, 0.2f, 0.2f);
        PrintCentered(GameSettings::levelLoadError.c_str(), screenH * 0.50f, 0.8f, 1.0f, 1.0f, 1.0f);
        PrintCentered("Press ESC to go back", screenH * 0.60f, 0.8f, 1.0f, 1.0f, 0.6f);
        return;
    }

    if (!grid) return;

    AEGfxSetBackgroundColor(0.05f, 0.05f, 0.05f);

    level.Draw(*grid, 1.0f);

    for (auto& t : activeTowers)
        t.Draw();

    //Because once bullets are using Graphics::DrawSprite(...), 
    //they’ll be rendered later by Graphics::RenderAll()
    //for (auto& b : activeBullets)
    //    b.Draw();

    for (Enemy* e : enemies)
    {
        if (e)
            e->Draw();
    }

    ParticleSystem::Draw();

    if (buildMergeSystem.IsDraggingTower())
        buildMergeSystem.DrawOverlay();

    shop.Draw();
    DrawUI();

    Graphics::RenderAll();

    //Reset
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // Only draw tutorial/codex when no blocking popup is covering them
    if (!IsBlockingPopupOpen())
    {
        m_tutorialPopup.Draw(m_uiFont);
        DrawCodex();
    }

    if (m_stageWon)
        DrawWinPopup();
    else if (gameOver)
        DrawLosePopup();
    else if (m_quitConfirmOpen)
        DrawQuitConfirmPopup();
    else if (m_pauseMenuOpen)
        DrawPausePopup();
}

void Scene_Prototype::Exit()
{
    if (m_codexTex)
    {
        AEGfxTextureUnload(m_codexTex);
        m_codexTex = nullptr;
    }

    m_tutorialPopup.Shutdown();
    shop.Exit();

    for (int i = (int)activeTowers.size() - 1; i >= 0; --i)
        buildMergeSystem.RemoveTowerAtIndex(i);
    activeTowers.clear();

    for (Enemy* e : enemies)
        delete e;
    enemies.clear();

    if (m_spriteSheet)
    {
        AEGfxTextureUnload(m_spriteSheet);
        m_spriteSheet = nullptr;
    }

    for (int i = 0; i < 3; ++i)
    {
        if (m_flagMeshes[i])
        {
            AEGfxMeshFree(m_flagMeshes[i]);
            m_flagMeshes[i] = nullptr;
        }
    }

    Graphics::Shutdown();
    ParticleSystem::Shutdown();
    level.Shutdown();
    DestroyGrid();
    buildMergeSystem.Shutdown();
}
#pragma endregion