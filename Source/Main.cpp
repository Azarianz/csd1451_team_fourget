// Solo Project 3 Port
// AUTHORS: Azarian

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "AEInput.h"
#include "Player.h"
#include "GameObject.h"
#include "HealthBar.h"
#include "Tower.h"
#include "GridSystem.h"


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

	// Init engine
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, true, NULL);
	AESysSetWindowTitle("Merge Defenders");

	GridSystem::Grid grid(16, 9, 100.0f, { -800.0f, -450.0f });
	grid.Init();

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

		// Render setup
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(1, 1, 1, 1);
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		// Draw
		shopTower.Draw();
		towerObj.Draw();
		grid.Update();	//Call Update: Which also calls Draw() in it and other execution order need for updating per frame

		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}

	// Cleanup
	player.Destroy();
	towerObj.Destroy();
	grid.Destroy();

	// free the system
	AESysExit();
}

