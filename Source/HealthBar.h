#pragma once
#include "GameObject.h"

struct HealthBar
{
    float value;
    float maxValue;

    void Init(float maxHP);
    void Update(const GameObject& player,
        const GameObject& cGreen,
        const GameObject& cRed,
        float dt);
    void Draw() const;
};
