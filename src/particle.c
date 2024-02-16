#include "raylib.h"
#include "raylib_extensions.h"
#include "raymath.h"
#include "sph.h"
#include <math.h>

// Initializes an array of particles with random positions and zero velocity
//
// Arguments:
// - particles: the array of particles to initialize
// - width: the width of the area in which to initialize the particles
// - height: the height of the area in which to initialize the particles
void particles_init_rand(struct particle_array *particles, float width,
                         float height) {
    for (int i = 0; i < particles->count; i++) {
        struct particle *p = &particles->items[i];
        p->position =
            (Vector2){GetRandomFloat(0, width), GetRandomFloat(0, height)};
        p->velocity = (Vector2){0.0f, 0.0f};
        p->density = 0.0f;
    }
}

// Initializes an array of particles in a grid pattern
//
// Note: the number of particles must be a perfect square
//
// Arguments:
// - particles: the array of particles to initialize
// - width: the width of the area in which to initialize the particles
// - height: the height of the area in which to initialize the particles
// - spacing: the distance between particles
void particles_init_grid(struct particle_array *particles, float width,
                         float height, float spacing) {
    int n = particles->count;
    int nx = (int)sqrtf(n);

    if (nx * nx != n) {
        SPH_LOG_WARN("Number of particles is not a perfect square, might "
                     "experience unexpected behavior");
    }

    float x_offset = (width - (nx - 1) * spacing) / 2.0f;
    float y_offset = (height - (nx - 1) * spacing) / 2.0f;

    for (int i = 0; i < nx; i++) {
        for (int j = 0; j < nx; j++) {
            int index = i * nx + j;
            struct particle *p = &particles->items[index];
            p->position =
                (Vector2){x_offset + j * spacing, y_offset + i * spacing};
            p->velocity = (Vector2){0.0f, 0.0f};
            p->density = 0.0f;
        }
    }
}

// Computes the density of particle i with respect to the other particles
//
// The function computes the density property of a particle by using the
// formula:
//
// A_i = sum_{j != i} m_j * A_j / rho_j * W(|x_i - x_j|, h)
//
// In this case A is the density, thus the formula becomes:
//
// rho_i = sum_{j != i} m_j * W(|x_i - x_j|, h)
//
// Where:
// - rho is the density of the particle (in kg/m^3)
// - m is the mass of the particle (in kg)
// - W is the kernel function
// - x is the position of the particle (in meters)
// - h is the smoothing length (in meters)
//
// Arguments:
// - particles: the array of particles
// - i: the index of the particle for which to compute the density
// - h: the smoothing length (in meters)
// - particle_mass: the mass of a particle (in kg)
// - type: the type of kernel function to use
//
// Returns the density of particle i (in kg/m^3)
float particle_density(struct particle_array *particles, int i, float h,
                       float particle_mass, enum kernel_type type) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 dir = Vector2Subtract(particles->items[i].position,
                                      particles->items[j].position);
        float x = Vector2Length(dir);
        float influence = kernel_function(x, h, type);
        density += influence * particle_mass;
    }

    return Max(density, 1e-6f);
}

// Computes the density of a point with respect to the particles
//
// Arguments:
// - particles: the array of particles
// - pos: the position of the point
// - h: the smoothing length (in meters)
// - particle_mass: the mass of a particle (in kg)
// - type: the type of kernel function to use
//
// Returns the density of the point (in kg/m^3)
float position_density(struct particle_array *particles, Vector2 pos, float h,
                       float particle_mass, enum kernel_type type) {
    float density = 0.0f;
    for (int j = 0; j < particles->count; j++) {
        Vector2 dir = Vector2Subtract(pos, particles->items[j].position);
        float x = Vector2Length(dir);
        float influence = kernel_function(x, h, type);
        density += influence * particle_mass;
    }

    return density;
}

// Computes the gradient of the pressure force of particle i with respect to the
// other particles
//
// The function computes the gradient of the pressure property by using the
// formula:
//
// grad A_i = -sum_{j != i} m_j * A_j / rho_j * grad W(|x_i - x_j|, h)
//
// In this case A is the pressure, thus the formula becomes:
//
// grad P_i = -sum_{j != i} m_j * P_j / rho_j * grad W(|x_i - x_j|, h)
//
// Where:
// - grad P is the gradient of the pressure of the particle (in N/m^2)
// - m is the mass of the particle (in kg)
// - P is the pressure of the particle (in N/m^2)
// - rho is the density of the particle (in kg/m^3)
// - grad W is the gradient of the kernel function
// - x is the position of the particle (in meters)
// - h is the smoothing length (in meters)
//
// Arguments:
// - particles: the array of particles
// - i: the index of the particle for which to compute the pressure force
// - h: the smoothing length (in meters)
// - particle_mass: the mass of a particle (in kg)
// - type: the type of kernel function to use
//
// Returns the gradient of the pressure force of particle i (in N/m^2)
Vector2 particle_pressure_gradient(struct particle_array *particles, int i,
                                   float h, float particle_mass,
                                   enum kernel_type kernel_type) {
    Vector2 force = {0.0f, 0.0f};
    for (int j = 0; j < particles->count; j++) {
        if (i == j) {
            continue;
        }

        Vector2 offset = Vector2Subtract(particles->items[j].position,
                                         particles->items[i].position);
        float x = Vector2Length(offset);
        Vector2 dir = Vector2Normalize(offset);

        float slope = kernel_function_derivative(x, h, kernel_type);
        float density = particles->items[j].density;
        float pressure = particles->items[j].pressure;
        float scale = 1.0 * pressure * slope * particle_mass / density;

        force = Vector2Add(force, Vector2Scale(dir, scale));
    }

    return force;
}
