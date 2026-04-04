#include "Scene_LevelEditor.h"

#include "SceneManager.h"
#include "AEEngine.h"
#include "AEInput.h"

SceneID Scene_LevelEditor::s_returnScene = SceneID::MainMenu;

#pragma region Helper
bool Scene_LevelEditor::IsInLeaveYesButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 160.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 175.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

bool Scene_LevelEditor::IsInLeaveNoButton(int mouseX, int mouseY) const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    const float popupW = 520.0f;
    const float popupH = 250.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    const float buttonW = 120.0f;
    const float buttonH = 28.0f;
    const float buttonX = centerX - buttonW * 0.5f;
    const float buttonY = top + 202.0f;

    return ((float)mouseX >= buttonX && (float)mouseX <= buttonX + buttonW &&
        (float)mouseY >= buttonY && (float)mouseY <= buttonY + buttonH);
}

void Scene_LevelEditor::UpdateLeaveConfirm()
{
    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    if (AEInputCheckTriggered(AEVK_LBUTTON))
    {
        if (IsInLeaveYesButton(mouseX, mouseY))
        {
            SceneManager::I().SwitchTo(s_returnScene);
            return;
        }

        if (IsInLeaveNoButton(mouseX, mouseY))
        {
            m_leaveConfirmOpen = false;
            return;
        }
    }

    if (AEInputCheckTriggered(AEVK_RETURN) || AEInputCheckTriggered(AEVK_SPACE))
    {
        SceneManager::I().SwitchTo(s_returnScene);
        return;
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        m_leaveConfirmOpen = false;
        return;
    }
}

void Scene_LevelEditor::DrawLeaveConfirm() const
{
    const float screenW = (float)AEGfxGetWindowWidth();
    const float screenH = (float)AEGfxGetWindowHeight();

    int mouseX = 0;
    int mouseY = 0;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    const bool hoverYes = IsInLeaveYesButton(mouseX, mouseY);
    const bool hoverNo = IsInLeaveNoButton(mouseX, mouseY);

    auto ToNdcX = [screenW](float px) { return (px / screenW) * 2.0f - 1.0f; };
    auto ToNdcY = [screenH](float py) { return 1.0f - (py / screenH) * 2.0f; };

    const float popupW = 620.0f;
    const float popupH = 220.0f;
    const float left = (screenW - popupW) * 0.5f;
    const float top = (screenH - popupH) * 0.5f;
    const float centerX = left + popupW * 0.5f;

    // Full dark overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
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

    if (editor.m_uiFont < 0)
        return;

    const char* titleText = "CONFIRM";
    const char* msgText = "ARE YOU SURE YOU WANT TO LEAVE?";
    const char* yesText = "YES";
    const char* noText = "NO";

    const float titleScale = 2.4f;
    const float msgScale = 1.6f;
    const float optScale = 1.2f;

    const float titleY = top + 80.0f;
    const float msgY = top + 120.0f;
    const float yesY = top + 185.0f;
    const float noY = top + 220.0f;

    float titleW = 160.0f, titleH = 160.0f;
    float msgW = 100.0f, msgH = 100.0f;
    float yesW = 60.0f, yesH = 60.0f;
    float noW = 60.0f, noH = 60.0f;

    AEGfxGetPrintSize((s8)editor.m_uiFont, titleText, titleScale, &titleW, &titleH);
    AEGfxGetPrintSize((s8)editor.m_uiFont, msgText, msgScale, &msgW, &msgH);
    AEGfxGetPrintSize((s8)editor.m_uiFont, yesText, optScale, &yesW, &yesH);
    AEGfxGetPrintSize((s8)editor.m_uiFont, noText, optScale, &noW, &noH);

    // Use the MESSAGE as the layout anchor
    const float blockLeft = centerX - msgW * 0.5f - 320.f;

    // title centered over the message block
    const float titleX = blockLeft + (msgW - titleW) * 0.5f + 180.f;

    // message uses the full block width
    const float msgX = blockLeft - 60.f;

    // YES / NO centered under the same block
    const float yesX = blockLeft + (msgW - yesW) * 0.5f + 280.f;
    const float noX = blockLeft + (msgW - noW) * 0.5f + 290.f;

    AEGfxPrint((s8)editor.m_uiFont, titleText,
        ToNdcX(titleX), ToNdcY(titleY),
        titleScale,
        1.0f, 1.0f, 0.2f, 1.0f);

    AEGfxPrint((s8)editor.m_uiFont, msgText,
        ToNdcX(msgX), ToNdcY(msgY),
        msgScale,
        1.0f, 1.0f, 1.0f, 1.0f);

    AEGfxPrint((s8)editor.m_uiFont, yesText,
        ToNdcX(yesX), ToNdcY(yesY),
        optScale,
        hoverYes ? 1.0f : 0.35f,
        hoverYes ? 1.0f : 0.35f,
        hoverYes ? 1.0f : 0.35f,
        1.0f);

    AEGfxPrint((s8)editor.m_uiFont, noText,
        ToNdcX(noX), ToNdcY(noY),
        optScale,
        hoverNo ? 1.0f : 0.35f,
        hoverNo ? 1.0f : 0.35f,
        hoverNo ? 1.0f : 0.35f,
        1.0f);
}
#pragma endregion


void Scene_LevelEditor::SetReturnScene(SceneID id)
{
    s_returnScene = id;
}

void Scene_LevelEditor::Init()
{
    editor.Init(20, 12); // grid size
    initialized = true;
    m_leaveConfirmOpen = false;
}

void Scene_LevelEditor::Update(float dt)
{
    if (!initialized)
        return;

    if (m_leaveConfirmOpen)
    {
        UpdateLeaveConfirm();
        return;
    }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        m_leaveConfirmOpen = true;
        return;
    }

    editor.Update(dt);
}

void Scene_LevelEditor::Draw()
{
    if (!m_leaveConfirmOpen)
        editor.Draw();
    else
        editor.Draw(); // keep map visible underneath if you want

    if (m_leaveConfirmOpen)
        DrawLeaveConfirm();
}

void Scene_LevelEditor::Exit()
{
    if (initialized)
    {
        editor.Shutdown();
        initialized = false;
        m_leaveConfirmOpen = false;
    }
}