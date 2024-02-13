#include "include/raylib_extensions.h"
#include "raylib.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PARTICLE_COUNT 1024 * 2
#define PARTICLE_RADIUS 2.0f

struct particle {
        Vector2 position;
        Vector2 velocity;
};

int main() {
    SetRandomSeed(time(NULL));

    struct particle particles[PARTICLE_COUNT];

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].position = (struct Vector2){
            GetRandomFloat(0, SCREEN_WIDTH), GetRandomFloat(0, SCREEN_HEIGHT)};
        particles[i].velocity = (struct Vector2){0.0f, 0.0f};
    }

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "water simulation");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            DrawCircleV(particles[i].position, PARTICLE_RADIUS, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
