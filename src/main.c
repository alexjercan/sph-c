#include "include/raylib_extensions.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// We are going to assume that the distance is measured in millimeters
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FROM_SCREEN_TO_WORLD(x) ((x) / 1000.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 1000.0f)

enum kernel_function_type {
    GAUSSIAN_KERNEL_FUNCTION,
    LAGUE_KERNEL_FUNCTION,
};

// The parameters for the simulation
struct simulation_parameters {
        int particle_count;        // Number of particles
        float particle_radius;     // Particle radius (in meters)
        float h;                   // Smoothing length (in meters)
        float particle_mass;       // Particle mass (in units of mass)
        float rest_density;        // Rest density (in kg/m^3)
        float adiabatic_index;     // Adiabatic index
        float speed_of_sound;      // Speed of sound (in m/s)
        float background_pressure; // Background pressure (in Pa)
        float epsilon;             // Epsilon
        float gravity;             // Gravity (in m/s^2)
        float damping;             // Collision with boundaries damping
        float pressure_multiplier; // Pressure multiplier
        enum kernel_function_type kernel_function_type; // Kernel function type
};

// Gaussian kernel function
//
// The Gaussian kernel function is used to compute the influence of a particle
// on another particle. It is defined as:
//
// W(x, h) = (1 / (h * sqrt(PI))) * exp(-x^2 / h^2)
//
// Where:
// - x is the distance between the two particles (in meters)
// - h is the smoothing length (in meters)
//
// Returns the influence of a particle on another particle (in 1/m)
float gaussian_kernel_function(float x, float h) {
    return (1.0f / (h * h * sqrtf(PI))) * expf(-1.0f * (x * x) / (h * h));
}

// Sebastian Lague's implementation of the kernel function
float lague_kernel_function(float x, float h) {
    float volume = PI * powf(h, 8) / 4.0f;
    float value = Max(0, (h * h - x * x));
    return value * value * value / volume;
}

// Kernel function
float kernel_function(float x, struct simulation_parameters params) {
    switch (params.kernel_function_type) {
    case GAUSSIAN_KERNEL_FUNCTION:
        return gaussian_kernel_function(x, params.h);
    case LAGUE_KERNEL_FUNCTION:
        return lague_kernel_function(x, params.h);
    }

    return 0.0f;
}

// Derivative of the Gaussian kernel function
//
// The derivative of the Gaussian kernel function is used to compute the
// gradient of the influence of a particle on another particle. It is defined
// as:
//
// dW(x, h) = (-2 / (h^2)) * W(x, h) * x
//
// Where:
// - x is the direction vector between the two particles (in meters)
// - h is the smoothing length (in meters)
//
// Returns the gradient of the influence of a particle on another particle (in
// 1/m^2)
float gaussian_kernel_function_derivative(float x, float h) {
    return (-2.0f * x) / (h * h) * gaussian_kernel_function(x, h);
}

// Sebastian Lague's implementation of the derivative of the kernel function
float lague_kernel_function_derivative(float x, float h) {
    if (x > h) {
        return 0.0f;
    }

    float f = h * h - x * x;
    float scale = -24.0 / (PI * powf(h, 8));
    return scale * x * f * f;
}

// Derivative of the kernel function
float kernel_function_derivative(float x, struct simulation_parameters params) {
    switch (params.kernel_function_type) {
    case GAUSSIAN_KERNEL_FUNCTION:
        return gaussian_kernel_function_derivative(x, params.h);
    case LAGUE_KERNEL_FUNCTION:
        return lague_kernel_function_derivative(x, params.h);
    }

    return 0.0f;
}

// The structure that represents a particle
struct particle {
        Vector2 position; // Position of the particle (in meters)
        Vector2 velocity; // Velocity of the particle (in m/s)
        float density;    // Density of the particle (in kg/m^3)
        float pressure;   // Pressure of the particle (in Pa)
};

// Initializes a particle
void particle_init(struct particle *p, Vector2 position) {
    p->position = position;
    p->velocity = (Vector2){0.0f, 0.0f};
}

// The structure that represents an array of particles
struct particle_array {
        struct particle *items;
        int count;
        int capacity;
};

// Initializes an array of particles
void particles_init(struct particle_array *particles) {
    for (int i = 0; i < particles->count; i++) {
        particle_init(
            &particles->items[i],
            (Vector2){GetRandomFloat(0, FROM_SCREEN_TO_WORLD(SCREEN_WIDTH)),
                      GetRandomFloat(0, FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT))});
    }
}

// Computes the density at a given point
float compute_density(struct particle_array *particles, Vector2 point,
                      struct simulation_parameters params) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        Vector2 dir = Vector2Subtract(point, particles->items[j].position);
        float x = Vector2Length(dir);
        float influence = kernel_function(x, params);
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
        float influence = gaussian_kernel_function(x, params.h);
        density += influence * params.particle_mass;
    }

    return density;
}

// Computes the pressure at a given point using the Cole equation
float compute_pressure_cole(struct particle_array *particles, int i,
                            struct simulation_parameters params) {
    float B = params.rest_density * params.speed_of_sound *
              params.speed_of_sound / params.adiabatic_index;
    float x = powf(particles->items[i].density / params.rest_density,
                   params.adiabatic_index) -
              1.0f;

    return B * x + params.background_pressure;
}

// Computes the pressure at a given point using the gas approximation
float compute_pressure_gas(struct particle_array *particles, int i,
                           struct simulation_parameters params) {
    float density_error = particles->items[i].density - params.rest_density;
    float pressure = density_error * params.pressure_multiplier;
    return pressure;
}

float compute_pressure(struct particle_array *particles, int i,
                       struct simulation_parameters params) {
    return compute_pressure_gas(particles, i, params);
}

Vector2 compute_momentum(struct particle_array *particles, int i,
                         struct simulation_parameters params) {
    Vector2 momentum = {0.0f, 0.0f};
    float pressure_a =
        particles->items[i].pressure /
        (particles->items[i].density * particles->items[i].density);
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        float pressure_b =
            particles->items[j].pressure /
            (particles->items[j].density * particles->items[j].density);
        Vector2 dir = Vector2Subtract(particles->items[i].position,
                                      particles->items[j].position);
        float x = Vector2Length(dir);
        float slope = kernel_function_derivative(x, params);
        Vector2 gradient = Vector2Scale(dir, slope);
        float pressure = pressure_a + pressure_b;

        Vector2 m =
            Vector2Scale(gradient, -1.0f * params.particle_mass * pressure);
        momentum = Vector2Add(momentum, m);
    }

    return momentum;
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
        float influence = kernel_function(x, params);
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
        particles->items[i].pressure = compute_pressure(particles, i, params);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 momentum = compute_momentum(particles, i, params);
        Vector2 gravity = {0.0f, params.gravity};
        momentum = Vector2Add(momentum, gravity);
        particles->items[i].velocity = Vector2Add(particles->items[i].velocity,
                                                  Vector2Scale(momentum, dt));
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 movement = compute_movement(particles, i, params);
        Vector2 position = Vector2Add(particles->items[i].position,
                                      Vector2Scale(movement, dt));

        if (position.x < 0) {
            position.x = 0;
            particles->items[i].velocity.x *= -1.0f * params.damping;
        } else if (position.x > FROM_SCREEN_TO_WORLD(SCREEN_WIDTH)) {
            position.x = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH);
            particles->items[i].velocity.x *= -1.0f * params.damping;
        }

        if (position.y < 0) {
            position.y = 0;
            particles->items[i].velocity.y *= -1.0f * params.damping;
        } else if (position.y > FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT)) {
            position.y = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT);
            particles->items[i].velocity.y *= -1.0f * params.damping;
        }

        particles->items[i].position = position;
    }
}

int main() {
    SetRandomSeed(time(NULL));

    struct simulation_parameters params = {
        .particle_count = 512,
        .particle_radius = 0.01f,
        .h = 0.1f,
        .particle_mass = 1.0f,
        .rest_density = 1000.0f,
        .adiabatic_index = 7.0f,
        .speed_of_sound = 1000.0f,
        .background_pressure = 1000000.0f,
        .epsilon = 0.9f,
        .gravity = 9.81f,
        .damping = 0.9f,
        .kernel_function_type = LAGUE_KERNEL_FUNCTION,
    };

    struct particle ps[params.particle_count];
    struct particle_array particles = {
        .items = ps,
        .count = params.particle_count,
        .capacity = params.particle_count,
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

        for (int i = 0; i < particles.count; i++) {
            Vector2 screen_position =
                (Vector2){FROM_WORLD_TO_SCREEN(particles.items[i].position.x),
                          FROM_WORLD_TO_SCREEN(particles.items[i].position.y)};
            float screen_radius = FROM_WORLD_TO_SCREEN(params.particle_radius);
            DrawCircleV(screen_position, screen_radius, BLUE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
