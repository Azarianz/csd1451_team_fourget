#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Enemy.h"
#include "Graphics.h"
#include <vector>
#include <string>

extern int nextTowerID;

// ============================================================
//  TowerHandler
//  Manages tower creation, shooting, projectiles, and the
//  shop tower used to spawn new towers.
// ============================================================
namespace TowerHandler {
    extern const int TOWER_SPRITE_COLS[6];  //AZA NEW ADDITION****

    // --------------------------------------------------------
    //  Enums
    // --------------------------------------------------------
    enum TowerType
    {
        BASIC_TOWER = 0,
        SNIPER_TOWER = 1,
        SLOW_TOWER = 2,
        RAPID_TOWER = 3,
        BOMB_TOWER = 4,
        BASE_TOWER = 5,
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
    //  Slow tower: slowPercent and slowDuration
    // --------------------------------------------------------
    struct LevelStats {
        float range;
        float fireCooldown;
        float damage;
        float speed;
        float slowPercent = 0.0f;
        float slowDuration = 0.0f;
    };
    extern LevelStats g_TowerLevelStats[6][3];
    // Reads tower stats for all types and levels from Assets/tower_stats.txt
    bool LoadTowerStatsFromFile(const std::string& filePath);

    struct BaseTowerStats {
		// can be overridden by file, but defaults to these values if file fails to load
        float health = 100.0f;
        float contactDamage = 10.0f;
        float sizeX = 80.0f;
        float sizeY = 80.0f;
    };
    extern BaseTowerStats g_BaseTowerStats;
    // Reads base tower stats from Assets/base_tower_stats.txt
    bool LoadBaseTowerStatsFromFile(const std::string& filePath);

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
        int purchaseCost     = 0;
 
        TowerDetails      details  {};
        Graphics::ShapeId spriteId = 0;

        static s8 g_StatsFont;          // font handle shared across all towers
        static void LoadStatsFont();    // loads the stats font, call once in LoadTowerAssets

        // --------------------------------------------------------
        //  Slow tower AOE attack (expanding ring, exclusive to 
        //  slow towers)
        // --------------------------------------------------------
        bool  aoeRingActive = false;
        std::vector<Enemy*> aoeHitList{};  // tracks enemies already hit this pulse
 
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
        void DrawTextBox() const;
 
        // Base-tower health
        bool TakeDamage(float dmg); // returns true if tower dies
        bool IsDead()       const;
        bool IsBaseTower()  const;
    };

    // Fires a projectile at a single enemy if in range and cooldown has expired.
    // Returns true if a shot was fired.
    bool TowerShoot(Tower& tower, Enemy& enemy, std::vector<ActiveBullet>& bullets);
    // AoE attack for slow towers. Damages and slows all enemies as the ring expands.
    // Must be called every frame while the tower is active.
    void SlowTowerAttack(Tower& tower, std::vector<Enemy*>& enemies);

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

    // Moves all active bullets, checks for hits, and removes expired or out of bounds bullets.
    void UpdateProjectiles(float dt, std::vector<Enemy*>& enemies,
        std::vector<ActiveBullet>& activeBullets);
    // Returns true if two circles overlap. Used for tower range, bullet hits and enemy collision.
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
		float lifeTime = 5.0f; // seconds until auto-expire (safety in case it misses and flies off)

        Enemy* target = nullptr; // homing target (nulled when enemy dies)
        bool   shouldRemove = false;

        void Update(float dt)
        {
			lifeTime -= dt;
            if (lifeTime <= 0.0f)
            {
                shouldRemove = true;
                return;
			}

			// If target is dead, stop homing and continue in same direction
            if (target && target->health <= 0.0f)
                target = nullptr;

            if (target)
            {
                float dx = target->x - x;
                float dy = target->y - y;
                float len = sqrtf(dx * dx + dy * dy);
                if (len > 0.0f)
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
        const float insetX = 0.5f / sheetW;  
        const float insetY = 0.5f / sheetH;  

        float w = 1.0f / totalCols;
        float h = 1.0f / totalRows;

        return {
            col       * w + insetX,   // u0
            row       * h + insetY,   // v0
            (col + 1) * w - insetX,   // u1
            (row + 1) * h - insetY,   // v1
        };
    }

    // --------------------------------------------------------
    //  Aza's Refactor Additions (Below Onwards)
    // --------------------------------------------------------
    // Main tower update. Ticks fire timers, triggers attacks and cleans up dead bullet targets.
    void UpdateTowerLogic(float dt,
        std::vector<Tower>& towers,
        std::vector<Enemy*>& enemies,
        std::vector<ActiveBullet>& activeBullets);

    // Creates and adds a base tower at the given position. Returns the index in the towers vector.
    int AddBaseTower(std::vector<Tower>& towers, float x, float y, float sizeX, float sizeY);
}


