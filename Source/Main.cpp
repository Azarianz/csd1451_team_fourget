// Solo Project 3 Port
// AUTHORS: Azarian

#include <crtdbg.h> // To check for memory leaks
#include <vector>
#include "AEEngine.h"
#include "AEInput.h"
#include "Player.h"
#include "GameObject.h"
#include "HealthBar.h"
#include "Tower.h"
#include "GridSystem.h"
#include "Utility.h"
#include "Shop.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int playerMoney = 500; // Starting currency
	Shop gameShop;

	// Container for towers bought from the shop
	std::vector<TowerHandler::Tower*> activeTowers;

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
	grid.InitScene();
	gameShop.Init(); // Loads tile_1.png, tile_2.png, etc.

	player.Init(0.0f, 0.0f, 50, 50, blue);
	shopTower.Init(600, 200, 50, 50, white);
	towerObj.Init(600, 200, 50, 50, blue);

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();

		float dt = (float)AEFrameRateControllerGetFrameTime();

		// Get mouse position for tower dragging
		float worldMX, worldMY;
		Utility::GetWorldMousePos(worldMX, worldMY);
		// Update Shop and handle tower purchase
		// Shop::Update returns a new Tower pointer if a slot was clicked
		TowerHandler::Tower* boughtTower = gameShop.Update(playerMoney);
		if (boughtTower) {
			activeTowers.push_back(boughtTower);
		}

		// Update all purchased towers (handles dragging and logic)
		for (auto t : activeTowers) {
			t->Update(worldMX, worldMY, *t);
		}

		// Rendering
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		// Draw all towers
		for (auto t : activeTowers) {
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			t->Draw();
		};

		// Update
		player.Update(dt);
		//shopTower.Update((float)mouseX, (float)mouseY, towerObj); //could use overloading for different update logic
		towerObj.Update((float)mouseX, (float)mouseY, towerObj);

		// Render setup
		AEGfxSetBackgroundColor(.5f, .5f, .5f);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND); // Required for PNG transparency

		// Draw
		shopTower.Draw();
		towerObj.Draw();
		grid.Update();	//Call Update: Which also calls Draw() in it and other execution order need for updating per frame
		gameShop.Draw();

		AESysFrameEnd();

		// check if forcing the application to quit
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;
	}
	
	// Cleanup
	player.Destroy();
	towerObj.Destroy();
	grid.Destroy();
	gameShop.Unload();

	for (auto t : activeTowers) {
		t->Destroy(); // Frees mesh
		delete t;     // Frees object memory
	}
	activeTowers.clear();

	// free the system
	AESysExit();
	return 1;
}

