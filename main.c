#include "raylib.h"
#include "raylib_extensions.h"
#include "raymath.h"
#include "sph.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: Implement the following functions:
// - compute_influence for a point and visualize it using a heatmap
// TODO: Should have this as a lib and use it in multiple examples/*.c files
// TODO: check for divisions by zero

// We are going to assume that the distance is measured in millimeters
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FROM_SCREEN_TO_WORLD(x) ((x) / 1000.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 1000.0f)

enum kernel_function_type {
    GAUSSIAN_KERNEL_FUNCTION,
    LAGUE_KERNEL_FUNCTION,
};

enum pressure_type {
    COLE_PRESSURE,
    GAS_PRESSURE,
};

// The parameters for the simulation
struct simulation_parameters {
        // World
        int particle_count; // Number of particles
        float gravity;      // Gravity (in m/s^2)
        float width;        // Width of the world (in meters)
        float height;       // Height of the world (in meters)

        // Particle
        float particle_radius; // Particle radius (in meters)
        float particle_mass;   // Particle mass (in units of mass)
        float damping;         // Collision with boundaries damping

        // Fluid
        float rest_density;               // Rest density (in kg/m^3)
        float adiabatic_index;            // Adiabatic index
        float speed_of_sound;             // Speed of sound (in m/s)
        float epsilon;                    // Epsilon
        float background_pressure;        // Background pressure (in Pa)
        float pressure_multiplier;        // Pressure multiplier
        enum pressure_type pressure_type; // Pressure type

        // Kernel function
        enum kernel_function_type kernel_function_type; // Kernel function type
        float h; // Smoothing length (in meters)
};

// Initializes an array of particles
void particles_init(struct particle_array *particles,
                    struct simulation_parameters params) {
    for (int i = 0; i < particles->count; i++) {
        struct particle *p = &particles->items[i];
        p->position = (Vector2){GetRandomFloat(0, params.width),
                                GetRandomFloat(0, params.height)};
        p->velocity = (Vector2){0.0f, 0.0f};
        p->density = 0.0f;
    }
}

// Kernel function
float kernel_function(float x, float h,
                      enum kernel_function_type kernel_function_type) {
    switch (kernel_function_type) {
    case GAUSSIAN_KERNEL_FUNCTION:
        return gaussian_kernel_function(x, h);
    case LAGUE_KERNEL_FUNCTION:
        return cubic_kernel_function(x, h);
    }

    return 0.0f;
}

// Derivative of the kernel function
float kernel_function_derivative(
    float x, float h, enum kernel_function_type kernel_function_type) {
    switch (kernel_function_type) {
    case GAUSSIAN_KERNEL_FUNCTION:
        return gaussian_kernel_function_derivative(x, h);
    case LAGUE_KERNEL_FUNCTION:
        return cubic_kernel_function_derivative(x, h);
    }

    return 0.0f;
}

// Computes the density at a given point
float compute_density(struct particle_array *particles, Vector2 point,
                      struct simulation_parameters params) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        Vector2 dir = Vector2Subtract(point, particles->items[j].position);
        float x = Vector2Length(dir);
        float influence =
            kernel_function(x, params.h, params.kernel_function_type);
        density += influence * params.particle_mass;
    }

    return density;
}

// Computes the particle density
float compute_particle_density(struct particle_array *particles, int i,
                               struct simulation_parameters params) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 dir = Vector2Subtract(particles->items[i].position,
                                      particles->items[j].position);
        float x = Vector2Length(dir);
        float influence =
            kernel_function(x, params.h, params.kernel_function_type);
        density += influence * params.particle_mass;
    }

    return density;
}

float compute_pressure(float density, struct simulation_parameters params) {
    switch (params.pressure_type) {
    case COLE_PRESSURE:
        return compute_pressure_cole(
            density, params.rest_density, params.speed_of_sound,
            params.adiabatic_index, params.background_pressure);
    case GAS_PRESSURE:
        return compute_pressure_gas(density, params.rest_density,
                                    params.pressure_multiplier);
    }
}

Vector2 compute_pressure_force(struct particle_array *particles, int i,
                               struct simulation_parameters params) {
    float density = particles->items[i].density;
    float pressure = compute_pressure(density, params);

    Vector2 force = {0.0f, 0.0f};
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 dir = Vector2Subtract(particles->items[i].position,
                                      particles->items[j].position);
        float x = Vector2Length(dir);
        float slope = kernel_function_derivative(x, params.h,
                                                 params.kernel_function_type);
        float scale = -1.0f * pressure * slope * params.particle_mass / density;

        force = Vector2Add(force, Vector2Scale(Vector2Normalize(dir), scale));
    }

    return force;
}

Vector2 compute_movement(struct particle_array *particles, int i,
                         struct simulation_parameters params) {
    Vector2 movement = {0.0f, 0.0f};
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 dir = Vector2Subtract(particles->items[i].position,
                                      particles->items[j].position);
        float x = Vector2Length(dir);
        float influence =
            kernel_function(x, params.h, params.kernel_function_type);
        float avg_density =
            (particles->items[i].density + particles->items[j].density) / 2.0f;
        Vector2 v_ba = Vector2Subtract(particles->items[j].velocity,
                                       particles->items[i].velocity);
        Vector2 v =
            Vector2Scale(v_ba, influence * params.particle_mass / avg_density);
        movement = Vector2Add(movement, v);
    }

    movement = Vector2Scale(movement, params.epsilon);
    movement = Vector2Add(movement, particles->items[i].velocity);

    return movement;
}

void simulation_step(struct particle_array *particles,
                     struct simulation_parameters params) {
    float dt = GetFrameTime();

    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density =
            compute_particle_density(particles, i, params);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 pressure_force = compute_pressure_force(particles, i, params);
        Vector2 pressure_acceleration =
            Vector2Scale(pressure_force, 1.0f / particles->items[i].density);

        Vector2 gravity_acceleration = {0.0f, params.gravity};

        Vector2 acceleration =
            Vector2Add(pressure_acceleration, gravity_acceleration);

        particles->items[i].velocity = Vector2Add(
            particles->items[i].velocity, Vector2Scale(acceleration, dt));
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 movement = compute_movement(particles, i, params);
        Vector2 position = Vector2Add(particles->items[i].position,
                                      Vector2Scale(movement, dt));

        if (position.x < 0) {
            position.x = 0;
            particles->items[i].velocity.x *= -1.0f * params.damping;
        } else if (position.x > params.width) {
            position.x = params.width;
            particles->items[i].velocity.x *= -1.0f * params.damping;
        }

        if (position.y < 0) {
            position.y = 0;
            particles->items[i].velocity.y *= -1.0f * params.damping;
        } else if (position.y > params.height) {
            position.y = params.height;
            particles->items[i].velocity.y *= -1.0f * params.damping;
        }

        particles->items[i].position = position;
    }
}

int main() {
    SetRandomSeed(time(NULL));

    struct simulation_parameters params = {
        .particle_count = 512,
        .gravity = 9.81f,
        .width = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH),
        .height = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT),
        .particle_radius = 0.01f,
        .particle_mass = 1.0f,
        .damping = 0.9f,
        .rest_density = 1000.0f,
        .adiabatic_index = 7.0f,
        .speed_of_sound = 1000.0f,
        .background_pressure = 1000000.0f,
        .pressure_multiplier = 1.0f,
        .pressure_type = GAS_PRESSURE,
        .epsilon = 0.9f,
        .kernel_function_type = LAGUE_KERNEL_FUNCTION,
        .h = 1.0f,
    };

    struct particle ps[params.particle_count];
    struct particle_array particles = {
        .items = ps,
        .count = params.particle_count,
        .capacity = params.particle_count,
    };

    particles_init(&particles, params);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Smoothed Particle Hydrodynamics");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            particles_init(&particles, params);
        }

        if (IsKeyDown(KEY_SPACE)) {
            simulation_step(&particles, params);
        }

        BeginDrawing();
        ClearBackground(DARKGRAY);

        for (int i = 0; i < particles.count; i++) {
            Vector2 screen_position =
                (Vector2){FROM_WORLD_TO_SCREEN(particles.items[i].position.x),
                          FROM_WORLD_TO_SCREEN(particles.items[i].position.y)};
            float screen_radius = FROM_WORLD_TO_SCREEN(params.particle_radius);
            DrawCircleV(screen_position, screen_radius, BLUE);
        }

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            params.h += GetMouseWheelMove() * 0.001f;
            if (params.h < 0.001f) {
                params.h = 0.001f;
            }

            Vector2 mouse_position = GetMousePosition();
            Vector2 world_position =
                (Vector2){FROM_SCREEN_TO_WORLD(mouse_position.x),
                          FROM_SCREEN_TO_WORLD(mouse_position.y)};
            float density = compute_density(&particles, world_position, params);
            DrawText(TextFormat("Density: %f kg/m^3", density), 10, 40, 20,
                     WHITE);

            float screen_radius = FROM_WORLD_TO_SCREEN(params.h);
            DrawCircleLinesV(GetMousePosition(), screen_radius, RED);

            DrawText(TextFormat("h: %f m", params.h), 10, 10, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
