#pragma once
#include "AEEngine.h"
#include "GameObject.h"

// Declare functions you want to use
struct Player : public GameObject
{
    float speed = 50.0f;

    void Init(float startX, float startY, float sizeX, float sizeY, Color c);
    void Update(float dt);
};