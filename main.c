#include "raylib.h"
#include "raymath.h"
#include "sph.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: Implement the following functions:
// - compute_influence for a point and visualize it using a heatmap
// TODO: check for divisions by zero
// Will need to figure out some good parameters but for that I need to use the
// ini file

// We are going to assume that the distance is measured in millimeters
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FROM_SCREEN_TO_WORLD(x) ((x) / 1000.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 1000.0f)

// This will move to sph.h with the pressure thing
enum pressure_type {
    COLE_PRESSURE,
    GAS_PRESSURE,
};

// The parameters for the simulation TODO use parser-ini for this
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
        enum kernel_type kernel_type; // Kernel function type
        float h;                      // Smoothing length (in meters)
};

// TODO: maybe use void * to make it more generic
float compute_pressure(float density, struct simulation_parameters params) {
    switch (params.pressure_type) {
    case COLE_PRESSURE:
        return pressure_cole(density, params.rest_density,
                             params.speed_of_sound, params.adiabatic_index,
                             params.background_pressure);
    case GAS_PRESSURE:
        return pressure_gas(density, params.rest_density,
                            params.pressure_multiplier);
    }
}

/*
// TODO: Think about this one and move it into sph
// this might also be possible to move into the particle.c file
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
*/

void simulation_step(struct particle_array *particles,
                     struct simulation_parameters params) {
    float dt = GetFrameTime();

    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density = particle_density(
            particles, i, params.h, params.particle_mass, params.kernel_type);
        particles->items[i].pressure =
            compute_pressure(particles->items[i].density, params);

        printf("Particle %d: density = %f, pressure = %f\n", i,
               particles->items[i].density, particles->items[i].pressure);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 pressure_gradient = particle_pressure_gradient(
            particles, i, params.h, params.particle_mass, params.kernel_type);

        printf("Particle %d: pressure_gradient = (%f, %f)\n", i,
               pressure_gradient.x, pressure_gradient.y);

        Vector2 screen_pressure_gradient =
            (Vector2){pressure_gradient.x, pressure_gradient.y};

        Vector2 screen_start_position = (Vector2){
            FROM_WORLD_TO_SCREEN(particles->items[i].position.x),
            FROM_WORLD_TO_SCREEN(particles->items[i].position.y),
        };
        Vector2 screen_end_position =
            Vector2Add(screen_start_position, screen_pressure_gradient);

        DrawLineV(screen_start_position, screen_end_position, GREEN);

        Vector2 pressure_acceleration =
            Vector2Scale(pressure_gradient, 1.0f / particles->items[i].density);

        Vector2 gravity_acceleration = {0.0f, params.gravity};

        Vector2 acceleration =
            Vector2Add(pressure_acceleration, gravity_acceleration);

        particles->items[i].velocity = Vector2Add(
            particles->items[i].velocity, Vector2Scale(acceleration, dt));
    }

    for (int i = 0; i < particles->count; i++) {
        // Vector2 movement = compute_movement(particles, i, params);
        Vector2 position =
            Vector2Add(particles->items[i].position,
                       Vector2Scale(particles->items[i].velocity, dt));

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
        .particle_radius = 0.005f,
        .particle_mass = 0.1f,
        .damping = 0.6f,
        .rest_density = 1000.0f,
        .adiabatic_index = 7.0f,
        .speed_of_sound = 1000.0f,
        .background_pressure = 1000000.0f,
        .pressure_multiplier = 0.01f,
        .pressure_type = GAS_PRESSURE,
        .epsilon = 0.9f,
        .kernel_type = GAUSSIAN_KERNEL,
        .h = 0.2f,
    };

    struct particle ps[params.particle_count];
    struct particle_array particles = {
        .items = ps,
        .count = params.particle_count,
        .capacity = params.particle_count,
    };

    particles_init_rand(&particles, params.width, params.height);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Smoothed Particle Hydrodynamics");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        if (IsKeyDown(KEY_R)) {
            particles_init_rand(&particles, params.width, params.height);
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

        if (IsKeyDown(KEY_SPACE)) {
            simulation_step(&particles, params);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
