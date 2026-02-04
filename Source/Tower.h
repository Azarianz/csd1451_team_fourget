#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include <vector>

namespace TowerHandler {

    static int nextTowerID = 0;

    enum TowerType{ BASIC_TOWER, SNIPER_TOWER};

    struct ProjectileType
    {
        AEGfxTexture* sprite;
        float damage;
        float speed;
    };

    struct TowerDetails {
        int level;
        int ID;
        float range;
        TowerType towerType;
        ProjectileType projectile;
    };

    struct Tower : public GameObject{
        int tower_count; //amount of towers
        bool isDragging;
        bool isSelected;
        float dragOffsetX;
        float dragOffsetY;
        TowerDetails details;

        void TowerInit(float xPos, float yPos, float xSize, float ySize, Color c, int segcount = 50);
        void TowerShoot();
        void Draw();
    };
    
    // DUMMY TOWER TO BASICALLY ACT AS SHOP TOWER SPRITE TO SELECT
    struct ShopTower : public GameObject {
        void ShopTowerInit(float startX, float startY, float sizeX, float sizeY, Color c, int segcount = 50);
        
    };
    void UpdateTowerSystem(float mouseX, float mouseY, ShopTower& shop, std::vector<Tower>& activeTowers);
}


