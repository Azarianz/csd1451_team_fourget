#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include <vector>

namespace TowerHandler {

    static int nextTowerID = 0;

    enum TowerType{ BASIC_TOWER, SNIPER_TOWER};

    struct ProjectileType
    {
        AEGfxTexture* sprite = nullptr;
        float damage = 0.0f;
        float speed = 0.0f;
    };

    struct TowerDetails {
        int level = 0;
        int ID = -1;
        float range = 0.0f;
        TowerType towerType = BASIC_TOWER;
        ProjectileType projectile{ nullptr, 0.0f, 0.0f };
    };

    struct Tower : public GameObject{
        int tower_count = 0; //amount of towers
        bool isDragging = false;
        bool isSelected = false;
        float dragOffsetX = 0.0f;
        float dragOffsetY = 0.0f;
        TowerDetails details; //dynamic array of tower details

        Tower()
            : GameObject()
            , tower_count(0)
            , isDragging(false)
            , isSelected(false)
            , dragOffsetX(0.0f)
            , dragOffsetY(0.0f)
            , details()
        {
        }

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


