#include "raylib.h"
#include "raylib_extensions.h"
#include "raymath.h"
#include "sph.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// TODO: update all the examples I guess
// TODO: Implement the following functions:
// - compute_influence for a point and visualize it using a heatmap
// TODO: check for divisions by zero
// Will need to figure out some good parameters but for that I need to use the
// ini file

// We are going to assume that the distance is measured in centimeters
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FROM_SCREEN_TO_WORLD(x) ((x) / 100.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 100.0f)

#define SCALE_FACTOR 25

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

void *get_pressure_params(struct simulation_parameters params) {
    void *pressure_params;
    switch (params.pressure_type) {
    case COLE_PRESSURE: {
        struct pressure_cole_params p = {
            .rest_density = params.rest_density,
            .speed_of_sound = params.speed_of_sound,
            .adiabatic_index = params.adiabatic_index,
            .background_pressure = params.background_pressure,
        };
        pressure_params = &p;
        break;
    }
    case GAS_PRESSURE: {
        struct pressure_gas_params p = {
            .rest_density = params.rest_density,
            .pressure_multiplier = params.pressure_multiplier,
        };
        pressure_params = &p;
        break;
    }
    }

    return pressure_params;
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
        float influence = kernel_function(x, params.h, params.kernel_type);
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

void resolve_collisions(struct particle *particle, Vector2 position,
                        struct simulation_parameters params) {
    if (position.x < 0) {
        position.x = 0;
        particle->velocity.x *= -1.0f * params.damping;
    } else if (position.x > params.width) {
        position.x = params.width;
        particle->velocity.x *= -1.0f * params.damping;
    }

    if (position.y < 0) {
        position.y = 0;
        particle->velocity.y *= -1.0f * params.damping;
    } else if (position.y > params.height) {
        position.y = params.height;
        particle->velocity.y *= -1.0f * params.damping;
    }

    particle->position = position;
}

void simulation_step(struct particle_array *particles,
                     struct simulation_parameters params) {
    float dt = GetFrameTime();

    for (int i = 0; i < particles->count; i++) {
        particles->items[i].density = particle_density(
            particles, i, params.h, params.particle_mass, params.kernel_type);
        particles->items[i].pressure =
            pressure_value(particles->items[i].density,
                           get_pressure_params(params), params.pressure_type);
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 pressure_gradient = particle_pressure_gradient(
            particles, i, params.h, params.particle_mass, params.kernel_type);

        Vector2 pressure_acceleration =
            Vector2Scale(pressure_gradient, 1.0f / particles->items[i].density);

        Vector2 gravity_acceleration = {0.0f, params.gravity};

        Vector2 acceleration =
            Vector2Add(pressure_acceleration, gravity_acceleration);

        particles->items[i].velocity =
            Vector2Scale(acceleration, dt); // Vector2Add(
                                            // particles->items[i].velocity, );
    }

    for (int i = 0; i < particles->count; i++) {
        // Vector2 movement = compute_movement(particles, i, params);
        Vector2 position = Vector2Add(particles->items[i].position,
                                      Vector2Scale(particles->items[i].velocity, dt));

        resolve_collisions(&particles->items[i], position, params);
    }
}

void DrawPressureTexture(struct particle_array *particles,
                         struct simulation_parameters params) {

    float pressure[SCREEN_WIDTH / SCALE_FACTOR][SCREEN_HEIGHT / SCALE_FACTOR] =
        {0};
    float max_pressure = 0.0f;
    for (int i = 0; i < particles->count; i++) {
        for (int x = 0; x < SCREEN_WIDTH; x += SCALE_FACTOR) {
            for (int y = 0; y < SCREEN_HEIGHT; y += SCALE_FACTOR) {
                Vector2 point =
                    (Vector2){FROM_SCREEN_TO_WORLD(x + SCALE_FACTOR / 2.0f),
                              FROM_SCREEN_TO_WORLD(y + SCALE_FACTOR / 2.0f)};
                float density =
                    position_density(particles, point, params.h,
                                     params.particle_mass, params.kernel_type);
                float p = pressure_value(density, get_pressure_params(params),
                                         params.pressure_type);
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

    struct simulation_parameters params = {
        .particle_count = 100,
        .gravity = 0.0f,
        .width = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH),
        .height = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT),
        .particle_radius = 0.05f,
        .particle_mass = 0.1f,
        .damping = 0.5f,
        .rest_density = 0.3f,
        .pressure_multiplier = 10.0f,
        .pressure_type = GAS_PRESSURE,
        .kernel_type = GAUSSIAN_KERNEL,
        .h = 1.0f,
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

        if (IsKeyDown(KEY_SPACE)) {
            simulation_step(&particles, params);
        }

        DrawPressureTexture(&particles, params);

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
