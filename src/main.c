#include "include/raylib_extensions.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PARTICLE_COUNT 1024 * 2
#define PARTICLE_RADIUS 20.0f

struct simulation_parameters {
    float h;
    float particle_mass;
};

struct particle {
    struct Vector2 position;
    float density;
};

void particle_init(struct particle *p, struct Vector2 position) {
    p->position = position;
    p->density = 0.0f;
}

struct particle_array {
    struct particle *items;
    int count;
    int capacity;
};

void particles_init(struct particle_array *particles) {
    for (int i = 0; i < particles->count; i++) {
        particle_init(&particles->items[i], (struct Vector2){
            GetRandomFloat(0, SCREEN_WIDTH), GetRandomFloat(0, SCREEN_HEIGHT)});
    }
}

float gaussian_kernel_function(float x, float h) {
    return (1.0f / (h * sqrtf(PI))) * expf(-1.0f * (x * x) / (h * h));
}

float compute_particle_density(struct particle_array *particles, int i, struct simulation_parameters params) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        float x = Vector2Distance(particles->items[i].position, particles->items[j].position);
        float influence = gaussian_kernel_function(x, params.h);
        density += influence * params.particle_mass;
    }

    return density;
}

void simulation_step(struct particle_array *particles, struct simulation_parameters params) {
    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density = compute_particle_density(particles, i, params);
    }
}

int main() {
    SetRandomSeed(time(NULL));

    struct simulation_parameters params = {
        .h = 100.0f,
        .particle_mass = 1.0f,
    };

    struct particle ps[PARTICLE_COUNT];
    struct particle_array particles = {
        .items = ps,
        .count = PARTICLE_COUNT,
        .capacity = PARTICLE_COUNT,
    };

    particles_init(&particles);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Smoothed Particle Hydrodynamics");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            particles_init(&particles);
        }

        simulation_step(&particles, params);

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            DrawCircleGradientV(particles.items[i].position, PARTICLE_RADIUS * particles.items[i].density, BLUE, BLANK);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
