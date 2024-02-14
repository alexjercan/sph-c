#include "include/raylib_extensions.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PARTICLE_COUNT 1024 * 2
#define PARTICLE_RADIUS 2.0f

int main() {
    SetRandomSeed(time(NULL));

    struct Vector2 particle_positions[PARTICLE_COUNT];

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        particle_positions[i] = (struct Vector2){
            GetRandomFloat(0, SCREEN_WIDTH), GetRandomFloat(0, SCREEN_HEIGHT)};
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Smoothed Particle Hydrodynamics");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            for (int i = 0; i < PARTICLE_COUNT; i++) {
                particle_positions[i] =
                    (struct Vector2){GetRandomFloat(0, SCREEN_WIDTH),
                                     GetRandomFloat(0, SCREEN_HEIGHT)};
            }
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            DrawCircleV(particle_positions[i], PARTICLE_RADIUS, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
