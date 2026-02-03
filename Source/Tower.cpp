#include "Tower.h"
#include "Utility.h"
#include <cmath>

namespace TowerHandler {
    void Tower::TowerInit(float xPos, float yPos, float xSize, float ySize, Color c, int seg_count) {
        //gameobj data
        x = xPos;
        y = yPos;
        _sizeX = xSize;
        _sizeY = ySize;
        segments = seg_count;
        color = c;
        mesh = nullptr; // don't build here

        //base tower data
        tower_count = 0;
        isDragging = false;
        dragOffsetX = 0;
        dragOffsetY = 0;

        details.level = 1;
        details.ID = nextTowerID++;
        details.range = 200.f;
        details.towerType = BASIC_TOWER;

    }


    void Tower::Draw() {

        // drawing range
        if (isDragging || isSelected) {
            // save original tower settings
            float originalTowerSizeX = _sizeX;
            float originalTowerSizeY = _sizeY;
            Color originalColor = color;

            //set new sizes for range
            _sizeX = _sizeY = details.range;
            color = { 1.f, 1.f, 1.f, 0.4f };

            // draw range
            GameObject::Draw();

            // restore original tower settings
            _sizeX = originalTowerSizeX;
            _sizeY = originalTowerSizeY;
            color = originalColor;
        }


        // draw tower itself
        GameObject::Draw();

    }

    void ShopTower::ShopTowerInit(float xPos, float yPos, float xSize, float ySize, Color c, int seg_count) {
        //gameobj data
        x = xPos;
        y = yPos;
        _sizeX = xSize;
        _sizeY = ySize;
        segments = seg_count;
        color = c;
        mesh = nullptr; // don't build here
    }

    void UpdateTowerSystem(float mouseX, float mouseY, ShopTower& shop, std::vector<Tower>& activeTowers) {
        Utility::GetWorldMousePos(mouseX, mouseY);

        // On trigger of left click to select topmost tower or spawn new tower
        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            // Order of logic
            // 1. Check for topmost tower first so mouse will select it first
            // 2. If topmost tower exist then return to exit early and not spawn a tower
            // 3. If topmost tower does not exist above the shop tower, spawn a tower

            // Find top most tower (highest tower id)
            Tower* topMostTower = nullptr;
            int highestID = -1;

            for (Tower& t : activeTowers) {
                float radius = t._sizeX;
                if (Utility::IsCircleClicked(t.x, t.y, radius, mouseX, mouseY)) {
                    // If this tower's ID is higher than our current best, update candidate
                    if (t.details.ID > highestID) {
                        highestID = t.details.ID;
                        topMostTower = &t;
                    }
                }
            }

            // Reset and deselect all towers after left click
            for (auto& t : activeTowers) {
                t.isSelected = false;
            }

            // If top most tower exists and is selected
            if (topMostTower != nullptr) {
                topMostTower->isSelected = true;
                topMostTower->isDragging = true;
                topMostTower->dragOffsetX = topMostTower->x - mouseX;
                topMostTower->dragOffsetY = topMostTower->y - mouseY;

                return; // top tower exists so it will drag it and not go through and spawn another tower
            }

            // Check if shop is clicked and spawn new tower
            if (Utility::IsCircleClicked(shop.x, shop.y, shop._sizeX, mouseX, mouseY)) {
                Tower newTower;

                // Initialize tower at shop position with its own parameters
                newTower.TowerInit(shop.x, shop.y, 55.0f, 55.0f, { 0.0f, 0.0f, 1.0f, 1.0f });

                // Force start dragging immediately
                newTower.isSelected = true;
                newTower.isDragging = true;
                newTower.dragOffsetX = 0; // set offset to 0 so it will center the tower to the mouse
                newTower.dragOffsetY = 0; // only done when tower is first spawned in

                activeTowers.push_back(newTower);
                return; // exit early to not trigger left click on the same frame again
            }
        }

        // Dragging tower (check through all towers to find the one that is selected (isDragging == true)
        for (Tower& t : activeTowers) {
            if (t.isDragging) {
                // Sustain drag offset so that middle of the tower does not keep snapping onto the mouse pos
                t.x = mouseX + t.dragOffsetX;
                t.y = mouseY + t.dragOffsetY;

                // On release of left click
                if (!AEInputCheckCurr(AEVK_LBUTTON)) {
                    t.isDragging = false;
                }
            }
        }
    }

    void Tower::TowerShoot() {

    }

}
