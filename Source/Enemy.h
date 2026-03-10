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

    // Pathfinding Data
    int pathIndex = 0;
    bool reachedEnd = false;

    // Static Sprite Data
    int spriteRow = 0;
    int spriteCol = 0;

    // Functions
    void Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed);
    void Update(float dt, const std::vector<Point>& path);

    // Helper to set static sprite data
    void SetSprite(int row, int col);

    // Override base GameObject draw
    void Draw();

    // Health Bar
    void DrawHealthBar() const;
};

// Specific Types
struct Zombie : public Enemy { void Init(); };
struct Skeleton : public Enemy { void Init(); };
struct Troll : public Enemy { void Init(); };
struct Golem : public Enemy { void Init(); };
struct Titan : public Enemy { void Init(); };

// --- Wave System ---
struct WaveData {
    int enemyType; // 0:Zombie, 1:Skeleton, 2:Troll, 3:Golem, 4:Titan
    int count;
    float spawnDelay;
};

struct WaveManager {
    std::vector<WaveData> waves;
    int currentWaveIndex = 0;
    int spawnedInCurrentWave = 0;
    float spawnTimer = 0.0f;
    bool waveComplete = true;

    bool LoadFromFile(const std::string& filename);
    Enemy* UpdateAndSpawn(float dt, const std::vector<Point>& path);
    int GetCurrentWaveNumber() const { return currentWaveIndex + 1; }
    int GetTotalWaves() const { return (int)waves.size(); }
    int GetTotalEnemiesInCurrentWave() const {
        if (currentWaveIndex < (int)waves.size()) return waves[currentWaveIndex].count;
        return 0;
    }
};