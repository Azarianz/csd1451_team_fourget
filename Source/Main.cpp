#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "GameSettings.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    GameSettings::Load();
    GameSettings::pendingRestart = false; // fresh launch, no pending restart

    const GameSettings::Resolution& res = GameSettings::RESOLUTIONS[GameSettings::resolutionIndex];

    AESysInit(hInstance, nCmdShow, res.width, res.height, 1, 60, true, NULL);
    AESysSetWindowTitle("Merge Defenders Prototype");

    // Pick which scene to boot (each teammate can change this line on their branch)
    SceneManager::I().Init(SceneID::MainMenu);   // <-- your editor scene

    int gGameRunning = 1;
    while (gGameRunning)
    {
        AESysFrameStart();
        float dt = (float)AEFrameRateControllerGetFrameTime();

        // --- scene hotkeys ---
        if (AEInputCheckTriggered(AEVK_1)) SceneManager::I().SwitchTo(SceneID::LevelEditor);
        if (AEInputCheckTriggered(AEVK_2)) SceneManager::I().SwitchTo(SceneID::LoadLevel);
        if (AEInputCheckTriggered(AEVK_3)) SceneManager::I().SwitchTo(SceneID::TowerTest);
        if (AEInputCheckTriggered(AEVK_4)) SceneManager::I().SwitchTo(SceneID::EnemyTest);
        if (AEInputCheckTriggered(AEVK_5)) SceneManager::I().SwitchTo(SceneID::ShopTest);
        if (AEInputCheckTriggered(AEVK_9)) SceneManager::I().SwitchTo(SceneID::Prototype);

        SceneID sceneThisFrame = SceneManager::I().Current();

        SceneManager::I().Update(dt);
        SceneManager::I().Draw();

        AESysFrameEnd();

        bool escapeHandledByScene = (sceneThisFrame == SceneID::Settings);
        if (GameSettings::quitGame ||
            (!escapeHandledByScene && AEInputCheckTriggered(AEVK_ESCAPE)) ||
            0 == AESysDoesWindowExist())
            gGameRunning = 0;
    }

    // Cleanup
    SceneManager::I().Exit();
    AESysExit();
}