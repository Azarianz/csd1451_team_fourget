#pragma once
#include "AEEngine.h"
#include "GameObject.h"


namespace TowerHandler {

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
        AEVec2 pos;
        ProjectileType projectile;
    };

    struct Tower : public GameObject
    {
        int tower_count; //amount of towers
        bool isDragging{false};
        float dragOffsetX{};
        float dragOffsetY{};
        TowerDetails* details; //dynamic array of tower details

        void Init(float startX, float startY, float sizeX, float sizeY, Color c, int segcount = 30);
        void Update(float mouseX, float mouseY, Tower &tower);
        void Draw();
        Tower* CreateTower(float startX, float startY, int towerID);
        void DestroyTower(Tower* tower);
    };

    
    
    
}


