#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include <vector> // Required for std::vector

// Helper struct for path points
struct Point { float x, y; };

struct Enemy : public GameObject
{


    // Enemy Stats
    float speed;
    float health;
    float maxhealth;
    float damage;

    // Pathfinding Data
    int pathIndex = 0;
    bool reachedEnd = false;

    // Functions
    void Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed);
    void Update(float dt, const std::vector<Point>& path);
};

// Specific Types
struct Zombie : public Enemy {
    void Init(); 
};
struct Skeleton : public Enemy { 
    void Init();
};
struct Troll : public Enemy { 
    void Init(); 
};
struct Golem : public Enemy {
    void Init();
};
struct Titan : public Enemy {
    void Init();
};