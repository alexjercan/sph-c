#include "raylib_extensions.h"
#include "rlgl.h"
#include <math.h>

float GetRandomFloat(float min, float max) {
    return min + GetRandomValue(0, 10000) / 10000.0f * (max - min);
}

void DrawCircleGradientV(Vector2 position, float radius, Color color1,
                         Color color2) {
    rlBegin(RL_TRIANGLES);
    for (int i = 0; i < 360; i += 10) {
        rlColor4ub(color1.r, color1.g, color1.b, color1.a);
        rlVertex2f(position.x, position.y);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a);
        rlVertex2f(position.x + cosf(DEG2RAD * (i + 10)) * radius,
                   position.y + sinf(DEG2RAD * (i + 10)) * radius);
        rlColor4ub(color2.r, color2.g, color2.b, color2.a);
        rlVertex2f(position.x + cosf(DEG2RAD * i) * radius,
                   position.y + sinf(DEG2RAD * i) * radius);
    }
    rlEnd();
}

float Max(float a, float b) { return a > b ? a : b; }
