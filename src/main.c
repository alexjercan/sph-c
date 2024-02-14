#include "include/raylib_extensions.h"
#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PARTICLE_COUNT 512
#define PARTICLE_RADIUS 0.1f

struct simulation_parameters {
    float h;
    float particle_mass;
    float rest_density;
    float adiabatic_index;
    float speed_of_sound;
    float background_pressure;
    float epsilon;
    float gravity;
    float damping;
};

struct particle {
    Vector2 position;
    Vector2 velocity;
    float density;
    float pressure;
};

void particle_init(struct particle *p, Vector2 position) {
    p->position = position;
    p->velocity = (Vector2){0.0f, 0.0f};
    p->density = 0.0f;
    p->pressure = 0.0f;
}

struct particle_array {
    struct particle *items;
    int count;
    int capacity;
};

void particles_init(struct particle_array *particles) {
    for (int i = 0; i < particles->count; i++) {
        particle_init(&particles->items[i], (Vector2){
            GetRandomFloat(0, SCREEN_WIDTH), GetRandomFloat(0, SCREEN_HEIGHT)});
    }
}

float gaussian_kernel_function(Vector2 x, float h) {
    float x_length = Vector2Length(x) / 1000.0f; // We imagine that the distance on our screen is in millimeters
    return (1.0f / (h * sqrtf(PI))) * expf(-1.0f * (x_length * x_length) / (h * h));
}

Vector2 gaussian_kernel_function_derivative(Vector2 x, float h) {
    return Vector2Scale(x, (-2.0f / (h * h)) * gaussian_kernel_function(x, h));
}

float compute_particle_density(struct particle_array *particles, int i, struct simulation_parameters params) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 x = Vector2Subtract(particles->items[i].position, particles->items[j].position);
        float influence = gaussian_kernel_function(x, params.h);
        density += influence * params.particle_mass;
    }

    return density;
}

float compute_pressure(struct particle_array *particles, int i, struct simulation_parameters params) {
    float B = params.rest_density * params.speed_of_sound * params.speed_of_sound / params.adiabatic_index;
    float x = powf(particles->items[i].density / params.rest_density, params.adiabatic_index) - 1.0f;

    return B * x + params.background_pressure;
}

Vector2 compute_momentum(struct particle_array *particles, int i, struct simulation_parameters params) {
    Vector2 momentum = {0.0f, 0.0f};
    float pressure_a = particles->items[i].pressure / (particles->items[i].density * particles->items[i].density);
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        float pressure_b = particles->items[j].pressure / (particles->items[j].density * particles->items[j].density);
        Vector2 x = Vector2Subtract(particles->items[i].position, particles->items[j].position);
        Vector2 gradient = gaussian_kernel_function_derivative(x, params.h);
        float pressure = pressure_a + pressure_b;

        Vector2 m = Vector2Scale(gradient, -1.0f * params.particle_mass * pressure);
        momentum = Vector2Add(momentum, m);
    }

    return momentum;
}

Vector2 compute_movement(struct particle_array *particles, int i, struct simulation_parameters params) {
    Vector2 movement = {0.0f, 0.0f};
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 x = Vector2Subtract(particles->items[i].position, particles->items[j].position);
        float influence = gaussian_kernel_function(x, params.h);
        float avg_density = (particles->items[i].density + particles->items[j].density) / 2.0f;
        Vector2 v_ba = Vector2Subtract(particles->items[j].velocity, particles->items[i].velocity);
        Vector2 v = Vector2Scale(v_ba, influence * params.particle_mass / avg_density);
        movement = Vector2Add(movement, v);
    }

    movement = Vector2Scale(movement, params.epsilon);
    movement = Vector2Add(movement, particles->items[i].velocity);

    return movement;
}

void simulation_step(struct particle_array *particles, struct simulation_parameters params) {
    float dt = 0.001f;

    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density = compute_particle_density(particles, i, params);
    }

    for (int i = 0; i < particles->count; i++) {
        particles->items[i].pressure = compute_pressure(particles, i, params);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 momentum = compute_momentum(particles, i, params);
        Vector2 gravity = {0.0f, params.gravity};
        momentum = Vector2Add(momentum, gravity);
        particles->items[i].velocity = Vector2Add(particles->items[i].velocity, Vector2Scale(momentum, dt));
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 movement = compute_movement(particles, i, params);
        Vector2 position = Vector2Add(particles->items[i].position, Vector2Scale(movement, dt));

        if (position.x < 0) {
            position.x = 0;
            particles->items[i].velocity.x *= -1.0f * params.damping;
        } else if (position.x > SCREEN_WIDTH) {
            position.x = SCREEN_WIDTH;
            particles->items[i].velocity.x *= -1.0f * params.damping;
        }

        if (position.y < 0) {
            position.y = 0;
            particles->items[i].velocity.y *= -1.0f * params.damping;
        } else if (position.y > SCREEN_HEIGHT) {
            position.y = SCREEN_HEIGHT;
            particles->items[i].velocity.y *= -1.0f * params.damping;
        }

        particles->items[i].position = position;
    }
}

int main() {
    SetRandomSeed(time(NULL));

    struct simulation_parameters params = {
        .h = 1.0f,
        .particle_mass = 1.0f,
        .rest_density = PARTICLE_COUNT / 2.0f,
        .adiabatic_index = 7.0f,
        .speed_of_sound = 1000.0f,
        .background_pressure = 1000000.0f,
        .epsilon = 0.9f,
        .gravity = 9810000.0f,
        .damping = 0.9f,
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

        float avg_density = 0.0f;
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            DrawCircleGradientV(particles.items[i].position, PARTICLE_RADIUS * particles.items[i].density, BLUE, BLANK);
            avg_density += particles.items[i].density;
        }
        avg_density /= PARTICLE_COUNT;
        printf("Average density: %f\n", avg_density);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
