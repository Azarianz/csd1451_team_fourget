#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"
#include "SceneManager.h"
#include "SceneID.h"
#include "GameSettings.h"
#include "AudioManager.h"

#include "AEMath.h"

namespace
{
    namespace
    {
        void RunSplashScreen()
        {
            AEGfxTexture* splashTex = AEGfxTextureLoad("Assets/DigiPen_Singapore_WEB_WHITE.png");
            if (!splashTex)
                return;

            const float minDuration = 3.0f;   // MUST show at least this long
            float splashTimer = 0.0f;

            while (AESysDoesWindowExist())
            {
                AESysFrameStart();

                float dt = (float)AEFrameRateControllerGetFrameTime();
                splashTimer += dt;

                // Fade-in (first 1.2s)
                float alpha = splashTimer / 1.2f;
                if (alpha > 1.0f) alpha = 1.0f;

                const float screenW = (float)AEGfxGetWindowWidth();
                const float screenH = (float)AEGfxGetWindowHeight();

                AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

                AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
                AEGfxTextureSet(splashTex, 0, 0);
                AEGfxSetBlendMode(AE_GFX_BM_BLEND);
                AEGfxSetTransparency(alpha);
                AEGfxSetColorToMultiply(1, 1, 1, 1);
                AEGfxSetColorToAdd(0, 0, 0, 0);

                AEMtx33 scaleM, rotM, transM, finalMtx;
                AEMtx33Scale(&scaleM, screenW * 0.60f, screenH * 0.32f);
                AEMtx33Rot(&rotM, 0.0f);
                AEMtx33Trans(&transM, 0.0f, 0.0f);

                AEMtx33Concat(&finalMtx, &rotM, &scaleM);
                AEMtx33Concat(&finalMtx, &transM, &finalMtx);
                AEGfxSetTransform(finalMtx.m);

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

                AESysFrameEnd();

                // exit ONLY after minimum time
                if (splashTimer >= minDuration)
                    break;
            }

            AEGfxTextureUnload(splashTex);
        }
    }
}

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

    AudioManager::Init();

    // Splash first
    RunSplashScreen();

    // Then boot main menu
    SceneManager::I().Init(SceneID::MainMenu);

    // Start global BGM once after splash + scene init
    float bgmVol = GameSettings::masterVolume / 100.0f;
    if (bgmVol < 0.0f) bgmVol = 0.0f;
    if (bgmVol > 1.0f) bgmVol = 1.0f;

    AudioManager::PlayBGM("Assets/bouken.mp3", bgmVol);
    AudioManager::SetBGMVolume(bgmVol);
    AudioManager::SetSFXVolume(1.f);

    int gGameRunning = 1;
    while (gGameRunning)
    {
        AESysFrameStart();
        float dt = (float)AEFrameRateControllerGetFrameTime();

        if (!AESysDoesWindowExist())
        {
            gGameRunning = 0;
            break;
        }

        SceneManager::I().Update(dt);
        SceneManager::I().Draw();

        AESysFrameEnd();

        if (SceneManager::I().Current() != SceneID::LevelEditor) {
            if (GameSettings::quitGame)
                gGameRunning = 0;
        }
    }

    AudioManager::Shutdown();
    SceneManager::I().Exit();
    AESysExit();
}