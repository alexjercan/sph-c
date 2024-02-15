#ifndef RAYLIB_EXTENSIONS_H
#define RAYLIB_EXTENSIONS_H

#include "raylib.h"

float GetRandomFloat(float min, float max);
void DrawCircleGradientV(Vector2 position, float radius, Color color1,
                         Color color2);
float Max(float a, float b);
Vector2 Vector2Random(float min, float max);

#endif // RAYLIB_EXTENSIONS_H
