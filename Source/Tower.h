#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include <vector>

namespace TowerHandler {

    static int nextTowerID = 0;

    enum TowerType{ BASIC_TOWER, SNIPER_TOWER, SLOW_TOWER, RAPID_TOWER};
	enum ProjectileType { BASIC_PROJECTILE, SNIPER_PROJECTILE, SLOW_PROJECTILE, RAPID_PROJECTILE };
    struct ShopTower;
    struct Tower;

    struct Projectile
    {
        AEGfxTexture* sprite = nullptr;
		ProjectileType projectileType = BASIC_PROJECTILE;
        float damage = 0.0f;
        float speed = 0.0f;
    };

    struct TowerDetails {
        int level = 0;
        int ID = -1;
        float range = 0.0f;
        TowerType towerType = BASIC_TOWER;
        Projectile projectile{};
    };

    struct Tower : public GameObject{
        int tower_count = 0; //amount of towers
        bool isDragging = false;
        bool isSelected = false;
        float dragOffsetX = 0.0f;
        float dragOffsetY = 0.0f;
        TowerDetails details; 

        Tower()
            : GameObject()
            , tower_count(0)
            , isDragging(false)
            , isSelected(false)
            , dragOffsetX(0.0f)
            , dragOffsetY(0.0f)
            , details()
        {}

        void TowerInit(float xPos, float yPos, float xSize, float ySize, ShopTower shopType, int segcount = 50);
        void TowerShoot(Tower& tower);
        void Draw();
    };
    
    // DUMMY TOWER TO BASICALLY ACT AS SHOP TOWER SPRITE TO SELECT
    struct ShopTower : public GameObject {
    private:
        TowerType shopTowerType = BASIC_TOWER;

    public:
        void ShopTowerInit(float startX, float startY, float sizeX, 
            float sizeY, TowerType towerType, int segcount = 50);
        TowerType const GetTowerType() { return shopTowerType; }
    };
    void UpdateTowerSystem(float mouseX, float mouseY, ShopTower& shop, std::vector<Tower>& activeTowers);
}


