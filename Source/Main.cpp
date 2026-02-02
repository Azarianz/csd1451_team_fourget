// Solo Project 3 Port
// AUTHORS: Azarian

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"
#include "Player.h"
#include "GameObject.h"
#include "HealthBar.h"
#include "Tower.h"


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
	Color white{ 1, 1, 1, 1};
	Player player;
	player.Init(0.0f, 0.0f, 50, 50, blue);

	//testing tower stuff
	TowerHandler::Tower shopTower;
	shopTower.Init(
		600, 200,
		50, 50,
		white
	);

	TowerHandler::Tower towerObj;
	towerObj.Init(
		600, 200,		// x, y pos
		50, 50,			// x, y scale
		blue			// color
	);

	int mouseX{}, mouseY{};
	AEInputGetCursorPosition(&mouseX, &mouseY);

	// Create game objects (circles)
	Color red{ 1, 0, 0, 1 };
	Color green{ 0, 1, 0, 1 };
	//GameObject cRed, cGreen;
	//cRed.Init(
	//	static_cast<float>(AEGfxGetWindowWidth() - 400), 
	//	static_cast<float>(AEGfxGetWindowHeight() / 2) - 100,
	//	200, 200, red);
	//cGreen.Init(
	//	400, 
	//	static_cast<float>(AEGfxGetWindowHeight() / 2) - 100,
	//	200, 200, green);

	// Create Healthbar
	/*HealthBar hp;
	hp.Init(100.0f);*/

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
		//shopTower.Update((float)mouseX, (float)mouseY, towerObj); //could use overloading for different update logic
		towerObj.Update((float)mouseX, (float)mouseY, towerObj);

		// overlap effects on player HP
		//hp.Update(player, cGreen, cRed, dt);

		// Render setup
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(1, 1, 1, 1);
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		// Draw
		//cRed.Draw();
		//cGreen.Draw();
		//player.Draw();
		shopTower.Draw();
		towerObj.Draw();

		// draw healthbar above player
		//hp.Draw();

		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}

	// Cleanup
	player.Destroy();
	//cRed.Destroy();
	//cGreen.Destroy();
	towerObj.Destroy();

	// free the system
	AESysExit();
}

