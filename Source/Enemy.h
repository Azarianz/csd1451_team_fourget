// Enemy.h
#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include <vector>
#include <string>

struct Point { float x, y; };

struct Enemy : public GameObject
{
    // Enemy Stats
    float speed = 0;
    float health = 0;
    float maxhealth = 1;
    float damage = 1;

    float healthRegenRate = 0.0f;

    // Slow state
    float slowMultiplier = 1.0f;  // 1.0 = normal, 0.7 = 30% slowed
    float slowTimer = 0.0f;  // counts down, resets multiplier on expiry
    float flashTimer = 0.0f;
    // Pathfinding Data
    int pathIndex = 0;
    bool reachedEnd = false;
    bool escapedBase = false;

    float facingX = 1.0f;

    // Static Sprite Data
    int spriteRow = 0;
    int spriteCol = 0;

    // Functions
    void Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed);
    void Scale(int waveNumber);
    void Update(float dt, const std::vector<Point>& path);

    // Helper to set static sprite data
    void SetSprite(int row, int col);

    // Override base GameObject draw
    void Draw();

    // Health Bar
    void DrawHealthBar() const;

    virtual int GetPoints() const { return 5; }
};

// Specific Types
struct Zombie : public Enemy { void Init(); int GetPoints() const override { return 10; } };
struct Skeleton : public Enemy { void Init(); int GetPoints() const override { return 20; } };
struct Troll : public Enemy { void Init(); int GetPoints() const override { return 30; } };
struct Golem : public Enemy { void Init(); int GetPoints() const override { return 50; } };
struct Titan : public Enemy { void Init(); int GetPoints() const override { return 40; } };
struct wavestarter : public Enemy { void Init(); int GetPoints() const override { return 5; } };


struct ZombieV1 : public Enemy { void Init(); int GetPoints() const override { return 5; } };
struct SkeletonV1 : public Enemy { void Init(); int GetPoints() const override { return 10; } };
struct TrollV1 : public Enemy { void Init(); int GetPoints() const override { return 15; } };
struct GolemV1 : public Enemy { void Init(); int GetPoints() const override { return 25; } };
struct TitanV1 : public Enemy { void Init(); int GetPoints() const override { return 20; } };

// --- Wave System ---
struct WaveData {
    int enemyType; // 0:Zombie, 1:Skeleton, 2:Troll, 3:Golem, 4:Titan, 5:wavestarter
    int count;
    float spawnDelay;
};

struct WaveManager {
    std::vector<WaveData> waves;
    int currentWaveIndex = 0;
    int spawnedInCurrentWave = 0;
    float spawnTimer = 0.0f;
    bool waveComplete = true;

    int wavestarterCount = 0;
    int totalWavestarters = 0;

    bool LoadFromFile(const std::string& filename);
    bool LoadLevel(int levelNumber);
    Enemy* UpdateAndSpawn(float dt, const std::vector<Point>& path);
    int GetCurrentWaveNumber() const { return currentWaveIndex + 1; }
    int GetTotalWaves() const { return (int)waves.size(); }
    int GetTotalEnemiesInCurrentWave() const {
        if (currentWaveIndex < (int)waves.size()) return waves[currentWaveIndex].count;
        return 0;
    }
    int GetWavestarterCount() const { return wavestarterCount; }
    int GetTotalWavestarters() const { return totalWavestarters; }

    // Add these two functions to track the timer
    float GetTimeUntilNextSpawn() const {
        if (currentWaveIndex < (int)waves.size()) {
            float remaining = waves[currentWaveIndex].spawnDelay - spawnTimer;
            return (remaining > 0.0f) ? remaining : 0.0f;
        }
        return 0.0f;
    }

    bool IsWaitingForWavestarter() const {
        if (currentWaveIndex < (int)waves.size()) {
            return waves[currentWaveIndex].enemyType == 5; // 5 is wavestarter
        }
        return false;
    }
};