#include "Tower.h"
#include "Utility.h"
#include "Enemy.h"
#include <cmath>
#include <algorithm>

namespace TowerHandler {
    void Tower::TowerInit(float xPos, float yPos, float xSize, float ySize, ShopTower shop, int seg_count) {
        //gameobj data
        x = xPos;
        y = yPos;
        _sizeX = xSize;
        _sizeY = ySize;
        segments = seg_count;
        mesh = nullptr; // don't build here

        //base tower data
        tower_count = 0;
        isDragging = false;
        dragOffsetX = 0;
        dragOffsetY = 0;

        details.level = 1;
        details.ID = nextTowerID++;
        details.towerType = shop.GetTowerType();
        details.fireTimer = 0.f;

        //base color, range, damage
        switch (details.towerType)
        {
        case TowerHandler::BASIC_TOWER:
            color = { 0.0f, 0.0f, 1.0f, 1.0f }; //blue
            details.range = 400.f;
            details.fireCooldown = 2.f;
            details.projectile.damage = 200.f;
            details.projectile.speed = 100.f;
            break;
        case TowerHandler::SNIPER_TOWER:
            color = { 0.0f, 1.0f, 0.0f, 1.0f }; //red
            details.range = 600.f;
            details.fireCooldown = 5.f;
            details.projectile.damage = 400.f;
            details.projectile.speed = 300.f;
            break;
        case TowerHandler::SLOW_TOWER:
            color = { 1.0f, 0.0f, 0.0f, 1.0f }; //green
            details.range = 200.f;
            details.fireCooldown = 2.5f;
            details.projectile.damage = 50.f;
            details.projectile.speed = 100.f;
            break;
        case TowerHandler::RAPID_TOWER:
            color = { 1.0f, 0.0f, 1.0f, 1.0f }; //purple
            details.range = 250.f;
            details.fireCooldown = 1.25f;
            details.projectile.damage = 200.f;
            details.projectile.speed = 100.f;
            break;
        default:
            // slow rof and white to show that its bugged if it ever reaches here
            color = { 1.0f, 1.0f, 1.0f, 1.0f }; //white
            details.range = 100.f;
            details.fireCooldown = 10.f;        //bad rof
            details.projectile.damage = 0.f;    //no damage
            details.projectile.speed = 100.f;    //bad projectile
            break;
        }
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

    void ShopTower::ShopTowerInit(float xPos, float yPos, float xSize, float ySize, TowerType towerType, int seg_count) {
        //gameobj data
        x = xPos;
        y = yPos;
        _sizeX = xSize;
        _sizeY = ySize;
        segments = seg_count;
        
        mesh = nullptr; // don't build here

        shopTowerType = towerType;
        switch (towerType)
        {
        case TowerHandler::BASIC_TOWER:
            color = { 0.0f, 0.0f, 1.0f, 1.0f }; //blue
            break;
        case TowerHandler::SNIPER_TOWER:
            color = { 0.0f, 1.0f, 0.0f, 1.0f }; //red
            break;
        case TowerHandler::SLOW_TOWER:
            color = { 1.0f, 0.0f, 0.0f, 1.0f }; //green
            break;
        case TowerHandler::RAPID_TOWER:
            color = { 1.0f, 0.0f, 1.0f, 1.0f }; //purple
            break;
        default:
            color = { 1.0f, 1.0f, 1.0f, 1.0f }; //white
            break;
        }
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
            for (Tower& t : activeTowers) {
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
                newTower.TowerInit(shop.x, shop.y, 55.0f, 55.0f, shop);

                // Force start dragging immediately
                newTower.isSelected = true;
                newTower.isDragging = true;
                newTower.dragOffsetX = 0; // set offset to 0 so it will center the tower to the mouse
                newTower.dragOffsetY = 0; // only done when tower is first spawned in

                activeTowers.push_back(newTower);
                return; // exit early to not trigger left click on the same frame again
            }

            for (Tower& t : activeTowers) {
                // Decrease the fire timer by the time passed since last frame
                if (t.details.fireTimer > 0.0f) {
					t.details.fireTimer -= 0.1f; //CHANGE TO DT LATER, JUST FOR TESTING PURPOSES
                }
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

    void TowerShoot(Tower& tower, Enemy& enemy, std::vector<ActiveBullet>& bullets) {

        if (tower.details.fireTimer > 0.0f) {
            return;
        }

        if (CircleCircleCollision(tower.x, tower.y, tower.details.range, enemy.x, enemy.y, enemy._sizeX)) {
			ActiveBullet newBullet;
            newBullet.x = tower.x;
            newBullet.y = tower.y;
            // Inside TowerShoot when creating newBullet:
            newBullet._sizeX = 10.0f;
            newBullet._sizeY = 10.0f;

            // Copy stats from the tower's projectile template
            newBullet.damage = tower.details.projectile.damage;
            newBullet.speed = tower.details.projectile.speed;
            newBullet.color = tower.color; // Match tower color for visual consistency

            newBullet.segments = 20;
            newBullet.mesh = nullptr;
            newBullet.target = &enemy;

            // Calculate Direction Vector
            float dx = enemy.x - tower.x;
            float dy = enemy.y - tower.y;
            float length = sqrtf(dx * dx + dy * dy);

            if (length > 0) {
                newBullet.dirX = dx / length; // Normalized X
                newBullet.dirY = dy / length; // Normalized Y
            }
            bullets.push_back(newBullet);
            tower.details.fireTimer = tower.details.fireCooldown;
        }
    }

	// bullet and enemy collision, both treated as circles for simplicity
    bool CircleCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2) {
		bool flag = false;
        float dx = x2 - x1;
		float dy = y2 - y1;
		float sqrDistance = dx * dx + dy * dy;
		float sqrRadiusSum = (r1 + r2) * (r1 + r2);

        if (sqrDistance <= sqrRadiusSum) {
			flag = true;
		}

		return flag;
	}

    void UpdateProjectiles(float dt, std::vector<Enemy*>& enemies,
        std::vector<ActiveBullet>& activeBullets){
        for (auto& b : activeBullets) {
            b.Update(dt);

            for (Enemy* e : enemies) {
                if (!e || e->health <= 0.0f) continue;

                if (CircleCircleCollision(b.x, b.y, b._sizeX, e->x, e->y, e->_sizeX)) {
                    e->health -= b.damage;
                    b.shouldRemove = true;
                    break;
                }
            }

            if (std::fabs(b.x) > 2000.f || std::fabs(b.y) > 2000.f)
                b.shouldRemove = true;
        }

        activeBullets.erase(
            std::remove_if(activeBullets.begin(), activeBullets.end(),
            [](const ActiveBullet& b) { return b.shouldRemove; }),
            activeBullets.end());
    }

}
