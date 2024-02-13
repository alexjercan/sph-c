#include "include/raylib_extensions.h"
#include "raylib.h"

float GetRandomFloat(float min, float max) {
    return min + GetRandomValue(0, 10000) / 10000.0f * (max - min);
}
