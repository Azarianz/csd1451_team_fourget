#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"
#include <cmath> // For sqrtf

void Enemy::Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed)
{
    // 1. Initialize the Parent GameObject
    // This sets x, y, _sizeX, _sizeY, and color for us.
    // We start at 0,0, but the Path logic will move us immediately.
    GameObject::Init(0.0f, 0.0f, sizeX, sizeY, c);

    // 2. Set Enemy Stats
    maxhealth = _hp;
    health = _hp;
    damage = _damage;
    speed = _speed;

    pathIndex = 0;
    reachedEnd = false;
}

void Enemy::Update(float dt, const std::vector<Point>& path)
{
    // Safety checks
    if (path.empty() || reachedEnd) return;
    if (pathIndex >= (int)path.size()) {
        reachedEnd = true;
        return;
    }

    // 1. Get Target
    Point target = path[pathIndex];

    // 2. Calculate Distance
    // Note: 'x' and 'y' here come from GameObject!
    float dx = target.x - x;
    float dy = target.y - y;
    float distSq = dx * dx + dy * dy;

    // 3. Move
    if (distSq > 25.0f) // If we are far away (> 5 pixels)
    {
        float dist = sqrtf(distSq);

        // Normalize and move
        x += (dx / dist) * speed * dt;
        y += (dy / dist) * speed * dt;
    }
    else
    {
        // 4. Reached Waypoint -> Go to next
        pathIndex++;
        if (pathIndex >= (int)path.size()) {
            reachedEnd = true;
        }
    }
}

// --- Specific Enemies ---

void Zombie::Init()
{
    // Green
    //          Size,   Size,   Colour,                 Health, Damage, Speed
    Enemy::Init(30.0f, 30.0f, { 0.0f, 1.0f, 0.0f, 1.0f }, 100.0f, 10.0f, 150.0f);
}

void Skeleton::Init()
{
    // Blue (Faster)
    Enemy::Init(20.0f, 20.0f, { 0.0f, 0.0f, 1.0f, 1.0f }, 60.0f, 5.0f, 250.0f);
}

void Troll::Init()
{
    // Red (Slower, Bigger)
    Enemy::Init(40.0f, 40.0f, { 1.0f, 0.0f, 0.0f, 1.0f }, 300.0f, 15.0f, 100.0f);
}
void Golem::Init()
{
    // PURPLE (Mix of Red + Blue)
    // R: 0.6f, G: 0.0f, B: 1.0f, Alpha: 1.0f
    Enemy::Init(60.0f, 60.0f, { 0.6f, 0.0f, 1.0f, 1.0f }, 300.0f, 40.0f, 50.0f);
}

void Titan::Init()
{
    // ORANGE (Full Red + Half Green)
    // R: 1.0f, G: 0.5f, B: 0.0f, Alpha: 1.0f
    Enemy::Init(50.0f, 50.0f, { 1.0f, 0.5f, 0.0f, 1.0f }, 250.0f, 30.0f, 75.0f);
}