#pragma once
#include "AEEngine.h"
#include "GameObject.h"

// Declare functions you want to use
struct Enemy : public GameObject
{

    float x, y;
    float width, height;


    float speed;
    float health;
    float maxhealth;
    float damage;

    void Init(float sizeX, float sizeY, Color c, float _hp, float _damage, float _speed);

    void Update(float dt);
};

struct Zombie : public Enemy
{
    void Init();
};

struct Skeleton : public Enemy
{
    void Init();
};

struct Troll : public Enemy
{
    void Init();
};
