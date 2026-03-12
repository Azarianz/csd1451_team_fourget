#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Enemy.h"
#include "Graphics.h"
#include <vector>

extern int nextTowerID;

namespace TowerHandler {

    enum TowerType{ BASIC_TOWER, SNIPER_TOWER, SLOW_TOWER, RAPID_TOWER, BASE_TOWER};
    struct ShopTower;
    struct Tower;
    struct ActiveBullet;

    struct LevelStats {
        float range;
        float fireCooldown;
        float damage;
        float speed;
    };

    // 3 levels for each tower type [level 0..2]
    const LevelStats TOWER_LEVEL_STATS[5][3] = {
        // BASIC_TOWER
        {
			// range, cooldown, damage, projectile speed
            { 400.f, 1.5f, 15.f, 400.f },  // Level 1
            { 500.f, 1.2f, 25.f, 450.f },  // Level 2
            { 600.f, 0.9f, 40.f, 500.f },  // Level 3
        },
        // SNIPER_TOWER
        {
            // range, cooldown, damage, projectile speed
            { 600.f, 3.0f,  50.f, 600.f }, // Level 1
            { 750.f, 2.5f,  80.f, 700.f }, // Level 2
            { 900.f, 2.0f, 120.f, 800.f }, // Level 3
        },
        // SLOW_TOWER
        {
            // range, cooldown, damage, projectile speed
            { 200.f, 2.0f,  5.f, 350.f },  // Level 1
            { 250.f, 1.7f, 10.f, 380.f },  // Level 2
            { 300.f, 1.4f, 18.f, 420.f },  // Level 3
        },
        // RAPID_TOWER
        {
            // range, cooldown, damage, projectile speed
            { 300.f, 1.0f, 10.f, 400.f },  // Level 1
            { 350.f, 0.7f, 16.f, 450.f },  // Level 2
            { 400.f, 0.5f, 24.f, 500.f },  // Level 3
        },
        // BASE_TOWER
        {
            { 0.f, 999999.f, 0.f, 0.f },
            { 0.f, 999999.f, 0.f, 0.f },
            { 0.f, 999999.f, 0.f, 0.f },
        },
    };

    struct Projectile
    {
        AEGfxTexture* sprite = nullptr;
        float damage = 0.0f;
        float speed = 0.0f;
    };

    struct TowerDetails {
        int level = 0;
        int ID = -1;
        float range = 0.0f;
		float fireCooldown = 0.0f;          // seconds between shots (constant for each tower type)
		float fireTimer = 0.f;              // seconds until next shot (counts down)
        TowerType towerType = BASIC_TOWER;
        Projectile projectile{};

        // Only for base tower
        float health = 0.0f;
        float maxHealth = 0.0f;
        float contactDamage = 0.0f;
        bool isBase = false;

        int spriteCol = 0;
        int spriteBaseRow = 0;
    };

    struct Tower : public GameObject{
        int tower_count = 0; //amount of towers
        bool isDragging = false;    
        bool isSelected = false;
        float dragOffsetX = 0.0f;
        float dragOffsetY = 0.0f;   
        TowerDetails details; 
        Graphics::ShapeId spriteId = 0;
        int sourceSlotIndex = -1;
        bool  isReturning = false;
        float returnTargetX = 0.0f;
        float returnTargetY = 0.0f;
        float pulseTimer = 0.0f;

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
        void Draw();
        void DrawHealthBar() const;
        void ApplyLevelStats();
        bool LevelUp(); // returns false if already max level
        bool TakeDamage(float dmg);
        bool IsDead() const;
        bool IsBaseTower() const;
    };
    bool TowerShoot(Tower& tower, Enemy& enemy, std::vector<ActiveBullet>& bullets);
    
    // SHOP TOWER STUFF
    struct ShopTower : public GameObject {
    private:
        TowerType shopTowerType = BASIC_TOWER;

    public:
        Graphics::ShapeId spriteId = 0;
        void ShopTowerInit(float startX, float startY, float sizeX, 
            float sizeY, TowerType towerType, int segcount = 50);
        void SetType(TowerType newType);
        TowerType const GetTowerType() { return shopTowerType; }
    };
    void UpdateTowerSystem(float mouseX, float mouseY, ShopTower& shop, std::vector<Tower>& activeTowers);
    bool CircleCircleCollision(float x1, float y1, float r1, float x2, float y2, float r2);

	// ACTIVE BULLET STUFF
    struct ActiveBullet : public GameObject {
        float damage = 0;
        float speed = 0;

        // Current movement direction (normalized)
        float dirX = 0, dirY = 0;

        // Homing target (OK in your scene because enemies are only deleted in Scene::Exit)
        Enemy* target = nullptr;

        bool shouldRemove = false;

        void Update(float dt) {
            if (target && target->health > 0.0f) {
                float dx = target->x - x;
                float dy = target->y - y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.0001f) {
                    dirX = dx / len;
                    dirY = dy / len;
                }
            }
            x += dirX * speed * dt;
            y += dirY * speed * dt;
        }
    };

    void UpdateProjectiles(float dt, std::vector<Enemy*>& enemies,
        std::vector<ActiveBullet>& activeBullets);


	// SPRITE UV CALCULATION
    struct UVRect { float u0, v0, u1, v1; };

    inline UVRect GetSpriteUV(int col, int row, int totalCols, int totalRows)
    {
        float w = 1.0f / totalCols;
        float h = 1.0f / totalRows;
        return { col * w, row * h, (col + 1) * w, (row + 1) * h };
    }
    void LoadTowerAssets();
}


