#include <crtdbg.h>
#include "AEEngine.h"
#include "AEInput.h"
#include "LevelEditor.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);
    AESysSetWindowTitle("Merge Defenders - Level Editor");

    // ---- Load Level Editor ----
    LevelEditor editor;
    editor.Init(20, 12); // change grid size here

    int gGameRunning = 1;
    while (gGameRunning)
    {
        AESysFrameStart();
        float dt = (float)AEFrameRateControllerGetFrameTime();

        editor.Update(dt);

        AESysFrameEnd();

        if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
            gGameRunning = 0;
    }

    editor.Shutdown();

    AESysExit();
}