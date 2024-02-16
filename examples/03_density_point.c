#include "raylib.h"
#include "raymath.h"
#include "sph.h"
#include <time.h>

#define FROM_SCREEN_TO_WORLD(x) ((x) / 100.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 100.0f)

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PARTICLE_COUNT 100
#define PARTICLE_MASS 0.1f
#define PARTICLE_RADIUS 0.1f
#define MIN_H 0.1f
#define MAX_H 2.5f
#define MIN_SPACING 0.1f
#define MAX_SPACING 1.0f

float compute_density(struct particle_array *particles, Vector2 point,
                      float h) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        Vector2 dir = Vector2Subtract(point, particles->items[j].position);
        float x = Vector2Length(dir);
        float influence = kernel_cubic(x, h);
        density += influence * PARTICLE_MASS;
    }

    return density;
}

void DrawDensityPoint(struct particle_array *particles, float h) {
    for (int i = 0; i < particles->count; i++) {
        Vector2 screen_position =
            (Vector2){FROM_WORLD_TO_SCREEN(particles->items[i].position.x),
                      FROM_WORLD_TO_SCREEN(particles->items[i].position.y)};
        float screen_radius = FROM_WORLD_TO_SCREEN(PARTICLE_RADIUS);
        DrawCircleV(screen_position, screen_radius, BLUE);
    }

    Vector2 mouse_position = GetMousePosition();
    Vector2 world_position = (Vector2){FROM_SCREEN_TO_WORLD(mouse_position.x),
                                       FROM_SCREEN_TO_WORLD(mouse_position.y)};
    float density = compute_density(particles, world_position, h);

    int text_x = mouse_position.x + 10;
    int text_y = mouse_position.y - 20;

    DrawText(TextFormat("Density: %f kg/m^3", density), text_x, text_y, 20,
             WHITE);

    float screen_radius = FROM_WORLD_TO_SCREEN(h);
    DrawCircleLinesV(GetMousePosition(), screen_radius, RED);
}

int main() {
    SetRandomSeed(time(NULL));

    SPH_LOG_INFO("Hold left mouse button and scroll to change the distance "
                 "between particles. Simply scroll to change the smoothing "
                 "length.");

    struct particle ps[PARTICLE_COUNT];
    struct particle_array particles = {
        .items = ps,
        .count = PARTICLE_COUNT,
        .capacity = PARTICLE_COUNT,
    };

    float width = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH);
    float height = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT);
    float spacing = (MIN_SPACING + MAX_SPACING) / 2.0f;

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Density Point");

    SetTargetFPS(60);
    float h = (MIN_H + MAX_H) / 2.0f;

    while (!WindowShouldClose()) {
        particles_init_grid(&particles, width, height, spacing);

        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            spacing += GetMouseWheelMove() * 0.1f;
            spacing = Clamp(spacing, MIN_SPACING, MAX_SPACING);
        } else {
            h += GetMouseWheelMove() * 0.25f;
            h = Clamp(h, MIN_H, MAX_H);
        }

        DrawDensityPoint(&particles, h);

        DrawText(TextFormat("h: %f m (scroll to change)", h), 10, 10, 20,
                 WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
