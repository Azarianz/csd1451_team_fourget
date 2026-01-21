#include "HealthBar.h"
#include "AEMath.h"

// ---------------- helpers ----------------
static float Clamp(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float Radius(const GameObject& o)
{
    // if you keep circles uniform, sizeX == sizeY, so this is fine
    return (o._sizeX < o._sizeY) ? o._sizeX : o._sizeY;
}


static bool Overlap(const GameObject& a, const GameObject& b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    float ra = Radius(a);
    float rb = Radius(b);

    float r = ra + rb;
    return (dx * dx + dy * dy) <= (r * r);
}

static void DrawRect(float cx, float cy, float w, float h)
{
    float hw = w * 0.5f;
    float hh = h * 0.5f;

    AEGfxMeshStart();
    unsigned int col = 0xFFFFFFFF;

    AEGfxTriAdd(
        cx - hw, cy - hh, col, 0, 0,
        cx + hw, cy - hh, col, 1, 0,
        cx + hw, cy + hh, col, 1, 1
    );

    AEGfxTriAdd(
        cx - hw, cy - hh, col, 0, 0,
        cx + hw, cy + hh, col, 1, 1,
        cx - hw, cy + hh, col, 0, 1
    );

    AEGfxVertexList* quad = AEGfxMeshEnd();

    AEMtx33 id;
    AEMtx33Identity(&id);
    AEGfxSetTransform(id.m);

    AEGfxMeshDraw(quad, AE_GFX_MDM_TRIANGLES);
    AEGfxMeshFree(quad);
}

// ---------------- API ----------------

void HealthBar::Init(float maxHP)
{
    maxValue = maxHP;
    value = maxHP;
}

void HealthBar::Update(const GameObject& player,
    const GameObject& cGreen,
    const GameObject& cRed,
    float dt)
{
    if (Overlap(player, cRed))
        value -= 40.0f * dt;   // DAMAGE

    if (Overlap(player, cGreen))
        value += 40.0f * dt;   // HEAL

    value = Clamp(value, 0.0f, maxValue);
}

void HealthBar::Draw() const
{
    float percent = value / maxValue;
    if (percent < 0.0f) percent = 0.0f;
    if (percent > 1.0f) percent = 1.0f;

    float width = 400.0f;
    float height = 20.0f;

    float x = 0.0f;
    float y = 350.0f;

    AEGfxSetColorToMultiply(0.2f, 0.2f, 0.2f, 1);
    DrawRect(x, y, width, height);

    float fillW = width * percent;

    AEGfxSetColorToMultiply(0, 1, 0, 1);
    DrawRect(x - (width - fillW) * 0.5f, y, fillW, height);
}

