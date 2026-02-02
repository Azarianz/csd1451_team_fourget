#include "Player.h"
#include "AEInput.h"
#include "AEMath.h"

void Player::Init(float startX, float startY, float sX, float sY, Color c)
{
    x = startX;
    y = startY;
    _sizeX = sX;
    _sizeY = sY;
    speed = 400.0f;
    segments = 64;
    color = c;
    mesh = nullptr; // don't build here
}

void Player::Update(float dt)
{
    if (AEInputCheckCurr(AEVK_W)) y += speed * dt;
    if (AEInputCheckCurr(AEVK_S)) y -= speed * dt;
    if (AEInputCheckCurr(AEVK_A)) x -= speed * dt;
    if (AEInputCheckCurr(AEVK_D)) x += speed * dt;
}