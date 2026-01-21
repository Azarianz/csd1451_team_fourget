// Solo Project 3 Port
// AUTHORS: Azarian

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"
#include "Player.h"
#include "GameObject.h"
#include "HealthBar.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int gGameRunning = 1;

	// - Initialization of your own variables go here -
	// Init Game Vars
	Color blue{ 0, 0, 1, 1 };
	Player player;
	player.Init(0.0f, 0.0f, 50, 50, blue);

	// Create game objects (circles)
	Color red{ 1, 0, 0, 1 };
	Color green{ 0, 1, 0, 1 };
	GameObject cRed, cGreen;
	cRed.Init(
		static_cast<float>(AEGfxGetWindowWidth() - 400), 
		static_cast<float>(AEGfxGetWindowHeight() / 2) - 100,
		200, 200, red);
	cGreen.Init(
		400, 
		static_cast<float>(AEGfxGetWindowHeight() / 2) - 100,
		200, 200, green);

	// Create Healthbar
	HealthBar hp;
	hp.Init(100.0f);

	// Init engine
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);
	AESysSetWindowTitle("Solo Project 1");

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();

		float dt = (float)AEFrameRateControllerGetFrameTime();

		// Update
		player.Update(dt);

		// overlap effects on player HP
		hp.Update(player, cGreen, cRed, dt);

		// Render setup
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(1, 1, 1, 1);
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		// Draw
		cRed.Draw();
		cGreen.Draw();
		player.Draw();

		// draw healthbar above player
		hp.Draw();

		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}

	// Cleanup
	player.Destroy();
	cRed.Destroy();
	cGreen.Destroy();

	// free the system
	AESysExit();
}

