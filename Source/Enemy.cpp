#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"
#include <cmath>

void Enemy::Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed)
{
    // 1. Initialize the Base GameObject
    // GameObject::Init(startX, startY, scaleX, scaleY, color)
    GameObject::Init(0.0f, 0.0f, sizeX, sizeY, c); //

    // 2. Initialize Enemy Specific Stats
    maxhealth = _hp;
    health = _hp;
    damage = _damage;
    speed = _speed;

    pathIndex = 0;
    reachedEnd = false;
}

void Enemy::Update(float dt, const std::vector<Point>& path)
{
    // If no path or finished, stop logic
    if (path.empty() || reachedEnd) return;

    // Check if we passed the end of the path
    if (pathIndex >= (int)path.size()) {
        reachedEnd = true;
        return;
    }

    // 1. Get Target
    Point target = path[pathIndex];

    // 2. Calculate Distance
    // Note: We use 'x' and 'y' directly (inherited from GameObject)
    float dx = target.x - x;
    float dy = target.y - y;
    float distSq = dx * dx + dy * dy;

    // 3. Move
    if (distSq > 25.0f) // If further than 5 pixels
    {
        float dist = sqrtf(distSq);

        // Normalize and move
        x += (dx / dist) * speed * dt;
        y += (dy / dist) * speed * dt;
    }
    else
    {
        // 4. Reached Waypoint -> Next
        pathIndex++;
        if (pathIndex >= (int)path.size()) {
            reachedEnd = true;
        }
    }

    // Note: We do NOT need to call GameObject::Update(dt) here 
    // because GameObject::Update is currently empty in your provided code.
}

// --- Specific Enemy Types ---

void Zombie::Init()
{
    // Green, Medium Speed
    Enemy::Init(30.0f, 30.0f, { 0.0f, 1.0f, 0.0f, 1.0f }, 100.0f, 10.0f, 100.0f);
}

void Skeleton::Init()
{
    // Blue, Fast Speed
    Enemy::Init(20.0f, 20.0f, { 0.0f, 0.0f, 1.0f, 1.0f }, 60.0f, 5.0f, 250.0f);
}

void Troll::Init()
{
    // Red, Slow Speed
    Enemy::Init(50.0f, 50.0f, { 1.0f, 0.0f, 0.0f, 1.0f }, 300.0f, 20.0f, 40.0f);
}