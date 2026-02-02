#include "Tower.h"
#include "Utility.h"
#include <cmath>

namespace TowerHandler {
    void Tower::Init(float xPos, float yPos, float xSize, float ySize, Color c, int seg_count) {
        x = xPos;
        y = yPos;
        _sizeX = xSize;
        _sizeY = ySize;
        segments = seg_count;
        color = c;
        mesh = nullptr; // don't build here

    }

    void Tower::Update(float mouseX, float mouseY, Tower &tower) {
        float radius = tower._sizeX;
        Utility::GetWorldMousePos(mouseX, mouseY); //convert mouse coordinates from screen coordinates to world coordinates
        
        //inital check to grab tower
        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
            if (Utility::IsCircleClicked(tower.x, tower.y, radius, mouseX, mouseY)) {
                tower.isDragging = true;
                
                //sustain offset to prevent snapping when dragging tower around
                tower.dragOffsetX = tower.x - mouseX;
                tower.dragOffsetY = tower.y - mouseY;
            }
        }

        //sustained drag
        if (tower.isDragging) {
            tower.x = mouseX + tower.dragOffsetX;
            tower.y = mouseY + tower.dragOffsetY;

            //released mouse button
            if (!AEInputCheckCurr(AEVK_LBUTTON)) {
                tower.isDragging = false;
            }
        }
    }

    void Tower::Draw() {

        //range

        GameObject::Draw();

        //tower itself
        GameObject::Draw();

    }

    Tower* Tower::CreateTower(float startX, float startY, int towerID) {
        Tower* newTower = new Tower;

        newTower->Init(startX, startY, _sizeX, _sizeY, color);
        newTower->isDragging = false;

        newTower->details = new TowerDetails;
        newTower->details->ID = towerID;
        newTower->details->level = 1;
        newTower->details->range = 150.f;
        newTower->details->pos = {startX, startY};

        return newTower;
    }

    void Tower::DestroyTower(Tower* tower) {
        if (!tower) return;

        if(tower->details)
            delete tower->details;

        delete tower;
    }
}
