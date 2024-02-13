#include "raylib.h"
#include <math.h>
#include <string.h>
#include <time.h>

#define G_CONST 9.81f * 100.0f
#define PARTICLE_WALL_COLLISION_DAMPING 0.5f
#define PARTICLE_COLLISION_DAMPING 0.1f
#define PARTICLE_VISCOSITY 0.9f
#define PARTICLE_SURFACE_TENSION 0.07f

#define SCREEN_WIDTH 800 * 2
#define SCREEN_HEIGHT 600 * 2

#define PARTICLE_COUNT 1024 * 2
#define PARTICLE_RADIUS 2.0f
#define PARTICLE_SPEED 100.0f

struct particle {
        Vector2 position;
        Vector2 velocity;
};

struct container {
        struct particle particles[PARTICLE_COUNT];
        Vector2 position;
        Vector2 size;
};

float GetRandomFloat(float min, float max) {
    return min + GetRandomValue(0, 10000) / 10000.0f * (max - min);
}

void container_init(struct container *container, Vector2 position,
                    Vector2 size) {
    container->position = position;
    container->size = size;

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        container->particles[i].position.x =
            GetRandomFloat(position.x, position.x + size.x);
        container->particles[i].position.y =
            GetRandomFloat(position.y, position.y + size.y);
        container->particles[i].velocity.x =
            GetRandomFloat(-PARTICLE_SPEED, PARTICLE_SPEED);
        container->particles[i].velocity.y =
            GetRandomFloat(-PARTICLE_SPEED, PARTICLE_SPEED);
    }
}

void container_update(struct container *container) {
    float dt = GetFrameTime();

    float x0 = container->position.x;
    float x1 = container->position.x + container->size.x;
    float y0 = container->position.y;
    float y1 = container->position.y + container->size.y;

    struct particle *particles = container->particles;
    struct particle new_particles[PARTICLE_COUNT];
    memcpy(new_particles, particles, PARTICLE_COUNT * sizeof(struct particle));

    for (int i = 0; i < PARTICLE_COUNT; i++) {
        struct particle *p = &particles[i];
        struct particle *np = &new_particles[i];

        // Apply collision with other particles
        for (int j = 0; j < PARTICLE_COUNT; j++) {
            if (i == j) {
                continue;
            }

            struct particle *p2 = &particles[j];
            float dx = p2->position.x - p->position.x;
            float dy = p2->position.y - p->position.y;

            float distance = sqrtf(dx * dx + dy * dy);

            if (distance < PARTICLE_RADIUS * 2.0f) {
                np->velocity.x -= p2->velocity.x * PARTICLE_COLLISION_DAMPING;
                np->velocity.y -= p2->velocity.y * PARTICLE_COLLISION_DAMPING;
            }
        }

        // Apply gravity
        np->velocity.y += G_CONST * dt;

        // Apply velocity
        np->position.x = p->position.x + np->velocity.x * dt;
        np->position.y = p->position.y + np->velocity.y * dt;

        // Check for collision with container
        if (np->position.x < x0) {
            np->position.x = x0;
            np->velocity.x = -np->velocity.x * PARTICLE_WALL_COLLISION_DAMPING;
        } else if (np->position.x > x1) {
            np->position.x = x1;
            np->velocity.x = -np->velocity.x * PARTICLE_WALL_COLLISION_DAMPING;
        }

        if (np->position.y < y0) {
            np->position.y = y0;
            np->velocity.y = -np->velocity.y * PARTICLE_WALL_COLLISION_DAMPING;
        } else if (np->position.y > y1) {
            np->position.y = y1;
            np->velocity.y = -np->velocity.y * PARTICLE_WALL_COLLISION_DAMPING;
        }
    }

    memcpy(particles, new_particles, PARTICLE_COUNT * sizeof(struct particle));
}

int main() {
    SetRandomSeed(time(NULL));

    struct container container;

    container_init(&container,
                   (struct Vector2){SCREEN_WIDTH / 4.0f, SCREEN_HEIGHT / 4.0f},
                   (struct Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f});

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "water simulation");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            container_init(
                &container,
                (struct Vector2){SCREEN_WIDTH / 4.0f, SCREEN_HEIGHT / 4.0f},
                (struct Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f});
        }

        container_update(&container);

        BeginDrawing();
        ClearBackground(BLACK);

        DrawRectangleV(container.position, container.size, DARKGRAY);
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            DrawCircleV(container.particles[i].position, PARTICLE_RADIUS, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
