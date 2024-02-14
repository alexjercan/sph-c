#include "raylib_extensions.h"
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
            p->position = (Vector2){x_offset + j * spacing, y_offset + i * spacing};
            p->velocity = (Vector2){0.0f, 0.0f};
            p->density = 0.0f;
        }
    }
}
