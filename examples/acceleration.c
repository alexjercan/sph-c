#include "raylib.h"
#include "raylib_extensions.h"
#include "raymath.h"
#include "sph.h"
#include <time.h>

#define FROM_SCREEN_TO_WORLD(x) ((x) / 100.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 100.0f)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PARTICLE_COUNT 100
#define PARTICLE_MASS 0.1f
#define PARTICLE_RADIUS 0.005f
#define MIN_H 0.1f
#define MAX_H 5.0f
#define KERNEL_TYPE CUBIC_KERNEL
#define GRAVITY 9.81f

void DrawPressureGradient(struct particle_array *particles, float h) {
    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density =
            particle_density(particles, i, h, PARTICLE_MASS, KERNEL_TYPE);
        particles->items[i].pressure =
            pressure_gas(particles->items[i].density, 1000.0f, 1.0f);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 screen_position =
            (Vector2){FROM_WORLD_TO_SCREEN(particles->items[i].position.x),
                      FROM_WORLD_TO_SCREEN(particles->items[i].position.y)};
        float screen_radius = FROM_WORLD_TO_SCREEN(h);

        DrawCircleGradientV(screen_position, screen_radius, WHITE, BLANK);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 screen_position =
            (Vector2){FROM_WORLD_TO_SCREEN(particles->items[i].position.x),
                      FROM_WORLD_TO_SCREEN(particles->items[i].position.y)};
        float screen_radius = FROM_WORLD_TO_SCREEN(PARTICLE_RADIUS);

        DrawCircleV(screen_position, screen_radius, BLUE);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 pressure_gradient = particle_pressure_gradient(
            particles, i, h, PARTICLE_MASS, KERNEL_TYPE);

        Vector2 pressure_acceleration =
            Vector2Scale(pressure_gradient, 1.0f / particles->items[i].density);

        Vector2 gravity_acceleration = {0.0f, GRAVITY};

        Vector2 acceleration =
            Vector2Add(pressure_acceleration, gravity_acceleration);

        Vector2 screen_acceleration = (Vector2){
            acceleration.x,
            acceleration.y,
        };

        Vector2 screen_start_position = (Vector2){
            FROM_WORLD_TO_SCREEN(particles->items[i].position.x),
            FROM_WORLD_TO_SCREEN(particles->items[i].position.y),
        };
        Vector2 screen_end_position =
            Vector2Add(screen_start_position, screen_acceleration);

        DrawLineV(screen_start_position, screen_end_position, GREEN);
    }

    float screen_radius = FROM_WORLD_TO_SCREEN(h);
    DrawCircleLinesV(GetMousePosition(), screen_radius, RED);
}

int main() {
    SetRandomSeed(time(NULL));

    struct particle ps[PARTICLE_COUNT];
    struct particle_array particles = {
        .items = ps,
        .count = PARTICLE_COUNT,
        .capacity = PARTICLE_COUNT,
    };

    float width = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH);
    float height = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT);

    particles_init_rand(&particles, width, height);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pressure Gradient View");

    SetTargetFPS(60);
    float h = (MIN_H + MAX_H) / 2.0f;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            particles_init_rand(&particles, width, height);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        h += GetMouseWheelMove() * 0.025f;
        h = Clamp(h, MIN_H, MAX_H);

        DrawPressureGradient(&particles, h);

        DrawText(TextFormat("h: %f m (scroll to change)", h), 10, 10, 20,
                 WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
