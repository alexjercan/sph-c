#include "raylib.h"
#include "raymath.h"
#include "sph.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

// We are going to assume that the distance is measured in centimeters
#define FROM_SCREEN_TO_WORLD(x) ((x) / 100.0f)
#define FROM_WORLD_TO_SCREEN(x) ((x) * 100.0f)

#define SCALE_FACTOR 25

struct simulation_parameters {
        // World
        long seed;          // Random seed
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
        float background_pressure;        // Background pressure (in Pa)
        float pressure_multiplier;        // Pressure multiplier
        enum pressure_type pressure_type; // Pressure type

        // Kernel function
        enum kernel_type kernel_type; // Kernel function type
        float h;                      // Smoothing length (in meters)
};

#define PARTICLE_COUNT 100
#define PARTICLE_CAPACITY 1000

struct simulation_parameters params = {
    .seed = 0,
    .particle_count = PARTICLE_COUNT,
    .gravity = 9.8f,
    .width = FROM_SCREEN_TO_WORLD(SCREEN_WIDTH),
    .height = FROM_SCREEN_TO_WORLD(SCREEN_HEIGHT),
    .particle_radius = 0.1f,
    .particle_mass = 0.2f,
    .damping = 0.9f,
    .rest_density = 0.8f,
    .pressure_multiplier = 100.0f,
    .pressure_type = GAS_PRESSURE,
    .kernel_type = GAUSSIAN_KERNEL,
    .h = 2.0f,
};
struct particle ps[PARTICLE_CAPACITY];
struct particle_array particles;

static void *get_pressure_params(struct simulation_parameters params) {
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

static void resolve_collisions(struct particle *particle, Vector2 position,
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

static void simulation_step(struct particle_array *particles,
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

        particles->items[i].velocity = Vector2Add(
            particles->items[i].velocity, Vector2Scale(acceleration, dt));
    }

    for (int i = 0; i < particles->count; i++) {
        Vector2 position =
            Vector2Add(particles->items[i].position,
                       Vector2Scale(particles->items[i].velocity, dt));

        resolve_collisions(&particles->items[i], position, params);
    }
}

void GameFrame(void)
{
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        Vector2 mouse_position = GetMousePosition();
        Vector2 world_position = {
            FROM_SCREEN_TO_WORLD(mouse_position.x),
            FROM_SCREEN_TO_WORLD(mouse_position.y),
        };
        struct particle p = {
            .position = world_position,
            .velocity = (Vector2){0.0f, 0.0f},
            .density = 0.0f,
            .pressure = 0.0f,
        };
        if (particles.count < particles.capacity) {
            particles.items[particles.count] = p;
            particles.count++;
        }
    }

    if (IsKeyReleased(KEY_R)) {
        particles.count = params.particle_count;
        particles_init_rand(&particles, params.width, params.height);
    }

    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        params.h += GetMouseWheelMove() * 0.1f;
        params.h = Clamp(params.h, 1.0f, 5.5f);
    } else if (IsKeyDown(KEY_LEFT_CONTROL)) {
        params.rest_density += GetMouseWheelMove() * 0.1f;
        params.rest_density = Clamp(params.rest_density, 0.1f, 3.5f);
    } else if (IsKeyDown(KEY_RIGHT_SHIFT)) {
        params.gravity += GetMouseWheelMove() * 0.5f;
        params.gravity = Clamp(params.gravity, -10.0f, 10.0f);
    }

    if (IsKeyDown(KEY_SPACE)) {
        simulation_step(&particles, params);
    }

    BeginDrawing();
    ClearBackground(DARKGRAY);

    // Draw particles
    for (int i = 0; i < particles.count; i++) {
        Vector2 screen_position =
            (Vector2){FROM_WORLD_TO_SCREEN(particles.items[i].position.x),
                      FROM_WORLD_TO_SCREEN(particles.items[i].position.y)};
        float screen_radius = FROM_WORLD_TO_SCREEN(params.particle_radius);
        DrawCircleV(screen_position, screen_radius, GREEN);
    }

    // Draw parameters
    DrawText(TextFormat("h: %f (left shift)", params.h), 10, 10, 20, WHITE);
    DrawText(TextFormat("rho: %f (left ctrl)", params.rest_density), 10, 30,
             20, WHITE);
    DrawText(TextFormat("g: %f (right shift)", params.gravity), 10, 50, 20,
             WHITE);

    EndDrawing();
}

void raylib_js_set_entry(void (*entry)(void));

int main() {
    unsigned int debug = 0;

    particles.items = ps;
    particles.count = params.particle_count;
    particles.capacity = PARTICLE_CAPACITY;

    particles_init_rand(&particles, params.width, params.height);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Smoothed Particle Hydrodynamics");

    SetTargetFPS(60);

    raylib_js_set_entry(GameFrame);

    return 0;
}
