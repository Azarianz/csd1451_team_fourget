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
	Color red{ 1, 0, 0, 1 };
	Color green{ 0, 1, 0, 1 };
	Color blue{ 0, 0, 1, 1 };
	Color white{ 1, 1, 1, 1};
	Player player;
	player.Init(0.0f, 0.0f, 50, 50, blue);

	//testing tower stuff
	std::vector<TowerHandler::Tower> towerList; // uninitialize tower list

	TowerHandler::ShopTower shopTower; // acts as a spawner for tower
	shopTower.ShopTowerInit(
		600, 200,
		50, 50,
		white
	);

	int mouseX{}, mouseY{};
	AEInputGetCursorPosition(&mouseX, &mouseY);


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
		UpdateTowerSystem(mouseX, mouseY, shopTower, towerList); // initializes and adds new tower to towerList

		// Render setup
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetColorToMultiply(1, 1, 1, 1);
		AEGfxSetColorToAdd(0, 0, 0, 0);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);
		AEGfxSetTransparency(1.0f);

		// Draw
		shopTower.Draw(); // draws shop tower
		for (TowerHandler::Tower& t : towerList) { // draw towers in towerList
			for (TowerHandler::Tower& t : towerList) {
				t.Draw();
			}
		}

		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}

	// Cleanup

	// free the system
	AESysExit();
}

