#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"

#include "SceneManager.h"
#include "SceneID.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);
    AESysSetWindowTitle("Merge Defenders Prototype");

    // Pick which scene to boot (each teammate can change this line on their branch)
    SceneManager::I().Init(SceneID::TowerTest);   // <-- your editor scene

     int gGameRunning = 1;
     while (gGameRunning)
     {
         AESysFrameStart();
         float dt = (float)AEFrameRateControllerGetFrameTime();

         // Update + Draw current scene
         SceneManager::I().Update(dt);
         SceneManager::I().Draw();

         AESysFrameEnd();

         if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
             gGameRunning = 0;
     }

	// Cleanup
     SceneManager::I().Exit();
	 AESysExit();
}
