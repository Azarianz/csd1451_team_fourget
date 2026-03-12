#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Enemy.h"
#include "Graphics.h"
#include <vector>

extern int nextTowerID;

// ============================================================
//  TowerHandler
//  Manages tower creation, shooting, projectiles, and the
//  shop tower used to spawn new towers.
// ============================================================
namespace TowerHandler {

    // --------------------------------------------------------
    //  Enums
    // --------------------------------------------------------
    enum TowerType
    {
        BASIC_TOWER = 0,
        SNIPER_TOWER = 1,
        SLOW_TOWER = 2,
        RAPID_TOWER = 3,
        BASE_TOWER = 4,
    };

    // --------------------------------------------------------
    //  Forward declarations
    // --------------------------------------------------------
    struct ShopTower;
    struct Tower;
    struct ActiveBullet;

    // --------------------------------------------------------
    //  Level stats table
    //  Indexed by [TowerType][level - 1]  (3 levels per type)
    //  Fields: range | fireCooldown | damage | projectileSpeed
    // --------------------------------------------------------
    struct LevelStats {
        float range;
        float fireCooldown;
        float damage;
        float speed;
    };

    // 3 levels for each tower type (except base)
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

    // --------------------------------------------------------
    //  Projectile  (template stored on a tower)
    // --------------------------------------------------------
    struct Projectile{
        float damage = 0.0f;
        float speed = 0.0f;
    };

    // --------------------------------------------------------
    //  TowerDetails  (stats + state for one tower)
    // --------------------------------------------------------
    struct TowerDetails{
        int       level        = 0;
        int       ID           = -1;
        float     range        = 0.0f;
        float     fireCooldown = 0.0f;  // seconds between shots (constant per type)
        float     fireTimer    = 0.0f;  // counts down to 0 before next shot
        TowerType towerType    = BASIC_TOWER;
        Projectile projectile  {};
 
        // Base-tower only
        float health        = 0.0f;
        float maxHealth     = 0.0f;
        float contactDamage = 0.0f;
        bool  isBase        = false;
 
        // Spritesheet position
        int spriteCol     = 0;
        int spriteBaseRow = 0;
    };


    // --------------------------------------------------------
    //  Tower
    // --------------------------------------------------------
    struct Tower : public GameObject{
        // Placement state
        bool  isDragging   = false;
        bool  isSelected   = false;
        bool  isPlaced     = false;
        float dragOffsetX  = 0.0f;
        float dragOffsetY  = 0.0f;
 
        // Misc
        int   tower_count    = 0;
        int   sourceSlotIndex = -1;
        bool  isReturning    = false;
        float returnTargetX  = 0.0f;
        float returnTargetY  = 0.0f;
        float pulseTimer     = 0.0f;
 
        TowerDetails      details  {};
        Graphics::ShapeId spriteId = 0;
 
        Tower()
            : GameObject()
            , isDragging(false)
            , isSelected(false)
            , dragOffsetX(0.0f)
            , dragOffsetY(0.0f)
            , tower_count(0)
            , details()
        {}
 
        // Core
        void TowerInit(float xPos, float yPos, float xSize, float ySize,
                       ShopTower shopType, int segcount = 50);
        void ApplyLevelStats();
        bool LevelUp();         // returns false if already at max level
 
        // Rendering
        void Draw();
        void DrawHealthBar() const;
 
        // Base-tower health
        bool TakeDamage(float dmg); // returns true if tower dies
        bool IsDead()       const;
        bool IsBaseTower()  const;
    };

    bool TowerShoot(Tower& tower, Enemy& enemy, std::vector<ActiveBullet>& bullets);
    
    // --------------------------------------------------------
    //  ShopTower  (the palette icon the player clicks to spawn)
    // --------------------------------------------------------
    struct ShopTower : public GameObject{
        Graphics::ShapeId spriteId = 0;
 
        void      ShopTowerInit(float startX, float startY, float sizeX,
                                float sizeY, TowerType towerType, int segcount = 50);
        void      SetType(TowerType newType);
        TowerType GetTowerType() const { return shopTowerType; }
 
    private:
        TowerType shopTowerType = BASIC_TOWER;
    };

    // --------------------------------------------------------
    //  System functions
    // --------------------------------------------------------
    void UpdateTowerSystem(float mouseX, float mouseY,
        ShopTower& shop, std::vector<Tower>& activeTowers);

    // Tower subsystems
    void SelectTopmostTower(float mouseX, float mouseY, 
        std::vector<Tower>& activeTowers);
    bool SpawnFromShop(float mouseX, float mouseY, ShopTower& shop, 
        std::vector<Tower>& activeTowers);
    void DragAndDropOnce(float mouseX, float mouseY, 
        std::vector<Tower>& activeTowers);

    void UpdateProjectiles(float dt, std::vector<Enemy*>& enemies,
        std::vector<ActiveBullet>& activeBullets);

    bool CircleCircleCollision(float x1, float y1, float r1,
        float x2, float y2, float r2);

    void LoadTowerAssets();

    // --------------------------------------------------------
    //  ActiveBullet  (homing projectile in flight)
    // --------------------------------------------------------
    struct ActiveBullet : public GameObject{
        float damage = 0.0f;
        float speed = 0.0f;

        float dirX = 0.0f;  // normalized movement direction
        float dirY = 0.0f;

        Enemy* target = nullptr; // homing target (nulled when enemy dies)
        bool   shouldRemove = false;

        void Update(float dt)
        {
            // Re-aim toward target if still alive
            if (target && target->health > 0.0f)
            {
                float dx = target->x - x;
                float dy = target->y - y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.0001f)
                {
                    dirX = dx / len;
                    dirY = dy / len;
                }
            }
            x += dirX * speed * dt;
            y += dirY * speed * dt;
        }
    };

    // --------------------------------------------------------
    //  Sprite UV helpers
    // --------------------------------------------------------
    struct UVRect { float u0, v0, u1, v1; };

    inline UVRect GetSpriteUV(int col, int row, int totalCols, int totalRows){
        // Spritesheet dimensions
        const float sheetW = 208.0f;
        const float sheetH = 160.0f;

        // Half-texel inset to prevent bleeding into adjacent sprites
        const float insetX = 0.5f / sheetW;  // 0.002404f
        const float insetY = 0.5f / sheetH;  // 0.003125f

        float w = 1.0f / totalCols;
        float h = 1.0f / totalRows;

        return {
            col       * w + insetX,   // u0
            row       * h + insetY,   // v0
            (col + 1) * w - insetX,   // u1
            (row + 1) * h - insetY,   // v1
        };
    }
}


