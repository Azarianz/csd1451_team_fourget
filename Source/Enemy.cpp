#include "Enemy.h"
#include "AEInput.h"
#include "AEMath.h"

const float FIXED_SPAWN_X = -300.0f;
const float FIXED_SPAWN_Y = 0.0f;

void Enemy::Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed)
{
    // FIX 1: Use local x/y/width/height (Instead of pos.x / scale.x)
    x = FIXED_SPAWN_X;
    y = FIXED_SPAWN_Y;

    width = sizeX;
    height = sizeY;

    // This works fine
    color = c;

    // Stats
    maxhealth = _hp;
    health = _hp;
    damage = _damage;
    speed = _speed;
}

void Enemy::Update(float dt)
{
    // FIX 2: Update local x
    x += speed * dt;
}

// --- Specific Enemies ---

void Zombie::Init()
{
    // FIX 3: Use curly braces { ... } instead of Color( ... )
    // This fixes "no instance of constructor" error
    Enemy::Init(20.0f, 20.0f, { 1.0f, 0.0f, 0.0f, 1.0f }, 100.0f, 1.0f, 50.0f);
}

void Skeleton::Init()
{
    // White Color
    Enemy::Init(15.0f, 15.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, 50.0f, 1.0f, 90.0f);
}

void Troll::Init()
{
    // Green Color
    Enemy::Init(40.0f, 40.0f, { 0.0f, 1.0f, 0.0f, 1.0f }, 200.0f, 1.0f, 25.0f);
}