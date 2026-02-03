#pragma once
#include "AEEngine.h"
#include "GameObject.h" //
#include <vector>

// Simple helper struct for the path (Point)
// Note: Color is already defined in GameObject.h, so we don't redefine it.
struct Point { float x, y; };

struct Enemy : public GameObject // Inherit from GameObject
{
    // --- Enemy Stats ---
    float speed;
    float health;
    float maxhealth;
    float damage;

    // --- Pathfinding ---
    int pathIndex = 0;
    bool reachedEnd = false;

    // --- Functions ---
    // We only need Init and Update. Draw/Destroy are handled by GameObject.
    void Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed);

    // Update takes the path so the enemy knows where to go
    void Update(float dt, const std::vector<Point>& path);
};

// Subtypes
struct Zombie : public Enemy { 
    void Init();
};
struct Skeleton : public Enemy {
    void Init(); 
};
struct Troll : public Enemy { 
    void Init(); 
};