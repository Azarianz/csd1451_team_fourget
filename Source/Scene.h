#pragma once

enum class SceneID
{
    None = 0,
    Gameplay_Test,
    Tower_Test,
    Grid_Test,

    // Teammates add more here:
    // Azar_Test,
    // Ryan_Test,
    // ...
};

class Scene
{
public:
    virtual ~Scene() = default;

    // Called once when entering the scene
    virtual void Init() {}

    // Called every frame (dt in seconds)
    virtual void Update(float dt) = 0;

    // Called every frame after Update()
    virtual void Draw() = 0;

    // Called once when leaving the scene
    virtual void Exit() {}
};