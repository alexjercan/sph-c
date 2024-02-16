#include "raylib.h"
#include "raylib_extensions.h"
#include "raymath.h"
#include "sph.h"
#include <time.h>

#define FROM_SCREEN_TO_WORLD(x) ((x) / 100.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 100.0f)

#define SCALE_FACTOR 25
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PARTICLE_COUNT 100
#define PARTICLE_MASS 0.1f
#define PARTICLE_RADIUS 0.1f
#define MIN_H 0.1f
#define MAX_H 2.5f
#define MIN_REST_DENSITY 0.1f
#define MAX_REST_DENSITY 3.0f
#define PRESSURE_MULTIPLIER 10.0f

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

float compute_pressure(float density, float rest_density) {
    return pressure_gas(density, rest_density, PRESSURE_MULTIPLIER);
}

void DrawPressureTexture(struct particle_array *particles, float h,
                         float rest_density) {

    float pressure[SCREEN_WIDTH / SCALE_FACTOR][SCREEN_HEIGHT / SCALE_FACTOR] =
        {0};
    float max_pressure = 0.0f;
    for (int i = 0; i < particles->count; i++) {
        for (int x = 0; x < SCREEN_WIDTH; x += SCALE_FACTOR) {
            for (int y = 0; y < SCREEN_HEIGHT; y += SCALE_FACTOR) {
                Vector2 point =
                    (Vector2){FROM_SCREEN_TO_WORLD(x + SCALE_FACTOR / 2.0f),
                              FROM_SCREEN_TO_WORLD(y + SCALE_FACTOR / 2.0f)};
                float density = compute_density(particles, point, h);
                float p = compute_pressure(density, rest_density);
                pressure[x / SCALE_FACTOR][y / SCALE_FACTOR] = p;
                max_pressure = fmaxf(max_pressure, fabsf(p));
            }
        }
    }

    Image img = GenImageColor(SCREEN_WIDTH / SCALE_FACTOR,
                              SCREEN_HEIGHT / SCALE_FACTOR, BLANK);
    for (int x = 0; x < SCREEN_WIDTH; x += SCALE_FACTOR) {
        for (int y = 0; y < SCREEN_HEIGHT; y += SCALE_FACTOR) {
            Color color;
            float p = pressure[x / SCALE_FACTOR][y / SCALE_FACTOR];
            float normalized_p = p / max_pressure;
            if (normalized_p > 0.1f) {
                float t = (normalized_p - 0.1f) / 0.9f;
                color = ColorGradient((Color){255, 0, 0, 255}, BLACK, t);
            } else if (normalized_p < -0.1f) {
                float t = -(normalized_p + 0.1f) / 0.9f;
                color = ColorGradient((Color){0, 0, 255, 255}, BLACK, t);
            } else if (normalized_p > 0.0f) {
                float t = normalized_p / 0.1f;
                color = ColorGradient(WHITE, (Color){255, 0, 0, 255}, t);
            } else {
                float t = -normalized_p / 0.1f;
                color = ColorGradient(WHITE, (Color){0, 0, 255, 255}, t);
            }

            ImageDrawPixel(&img, x / SCALE_FACTOR, y / SCALE_FACTOR, color);
        }
    }
    ImageResize(&img, SCREEN_WIDTH, SCREEN_HEIGHT);

    Texture2D texture = LoadTextureFromImage(img);
    DrawTexture(texture, 0, 0, WHITE);
    UnloadImage(img); // Unload the image as the texture now holds the data
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
    particles_init_rand(&particles, width, height);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Density Point");

    SetTargetFPS(60);
    float h = (MIN_H + MAX_H) / 2.0f;
    float rest_density = (MIN_REST_DENSITY + MAX_REST_DENSITY) / 2.0f;

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            particles_init_rand(&particles, width, height);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            h += GetMouseWheelMove() * 0.1;
            h = Clamp(h, MIN_H, MAX_H);
        }
        if (IsKeyDown(KEY_LEFT_CONTROL)) {
            rest_density += GetMouseWheelMove() * 0.1;
            rest_density =
                Clamp(rest_density, MIN_REST_DENSITY, MAX_REST_DENSITY);
        }

        DrawPressureTexture(&particles, h, rest_density);

        // Draw the smoothing length
        Vector2 mouse_position = GetMousePosition();
        Vector2 world_position =
            (Vector2){FROM_SCREEN_TO_WORLD(mouse_position.x),
                      FROM_SCREEN_TO_WORLD(mouse_position.y)};

        float screen_radius = FROM_WORLD_TO_SCREEN(h);
        DrawCircleLinesV(GetMousePosition(), screen_radius, RED);

        // Draw the particles
        for (int i = 0; i < particles.count; i++) {
            Vector2 screen_position =
                (Vector2){FROM_WORLD_TO_SCREEN(particles.items[i].position.x),
                          FROM_WORLD_TO_SCREEN(particles.items[i].position.y)};
            float screen_radius = FROM_WORLD_TO_SCREEN(PARTICLE_RADIUS);
            DrawCircleV(screen_position, screen_radius, BLUE);
        }

        DrawText(TextFormat("h: %f m (shift scroll)", h), 10, 10, 20, WHITE);
        DrawText(TextFormat("rho: %f kg/m^3 (ctrl scroll)", rest_density), 10,
                 40, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
