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

Vector2 Vector2Random(float min, float max) {
    return (Vector2){GetRandomFloat(min, max), GetRandomFloat(min, max)};
}

Color ColorGradient(Color start, Color end, float t) {
    return (Color){
        (unsigned char)(start.r + (end.r - start.r) * t),
        (unsigned char)(start.g + (end.g - start.g) * t),
        (unsigned char)(start.b + (end.b - start.b) * t),
        (unsigned char)(start.a + (end.a - start.a) * t),
    };
}
