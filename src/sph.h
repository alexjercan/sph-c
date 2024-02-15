#ifndef SPH_H
#define SPH_H

#include "raylib.h"

#ifndef SPH_NO_STDIO
#include <stdio.h>
#endif

#ifdef DS_QUIET
#define DS_NO_LOG_ERROR
#define DS_NO_LOG_WARN
#define DS_NO_LOG_INFO
#endif

#ifdef SPH_NO_TERMINAL_COLORS
#define SPH_TERMINAL_RED ""
#define SPH_TERMINAL_YLW ""
#define SPH_TERMINAL_BLU ""
#define SPH_TERMINAL_RST ""
#else
#define SPH_TERMINAL_RED "\033[1;31m"
#define SPH_TERMINAL_YLW "\033[1;33m"
#define SPH_TERMINAL_BLU "\033[1;34m"
#define SPH_TERMINAL_RST "\033[0m"
#endif

#if defined(SPH_NO_STDIO) || defined(DS_NO_LOG_INFO)
#define SPH_LOG_INFO(format, ...)
#else
#define SPH_LOG_INFO(format, ...)                                              \
    fprintf(stdout,                                                            \
            SPH_TERMINAL_BLU "INFO" SPH_TERMINAL_RST ": %s:%d: " format "\n",  \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(SPH_NO_STDIO) || defined(DS_NO_LOG_WARN)
#define SPH_LOG_WARN(format, ...)
#else
#define SPH_LOG_WARN(format, ...)                                              \
    fprintf(stdout,                                                            \
            SPH_TERMINAL_YLW "WARN" SPH_TERMINAL_RST ": %s:%d: " format "\n",  \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(SPH_NO_STDIO) || defined(DS_NO_LOG_ERROR)
#define SPH_LOG_ERROR(format, ...)
#else
#define SPH_LOG_ERROR(format, ...)                                             \
    fprintf(stderr,                                                            \
            SPH_TERMINAL_RED "ERROR" SPH_TERMINAL_RST ": %s:%d: " format "\n", \
            __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#ifndef SPH_EXPORT
#define SPH_EXPORT
#endif

// The structure that represents a particle
struct particle {
        Vector2 position; // Position of the particle (in meters)
        Vector2 velocity; // Velocity of the particle (in m/s)
        float density;    // Density of the particle (in kg/m^3)
        float pressure;   // Pressure of the particle (in Pa)
};

// The structure that represents an array of particles
struct particle_array {
        struct particle *items;
        int count;
        int capacity;
};

enum kernel_type {
    GAUSSIAN_KERNEL,
    CUBIC_KERNEL,
};

#if defined(__cplusplus)
extern "C" { // Prevents name mangling of functions
#endif

// Particles
SPH_EXPORT void particles_init_grid(struct particle_array *particles,
                                    float width, float height, float spacing);
SPH_EXPORT void particles_init_rand(struct particle_array *particles,
                                    float width, float height);
SPH_EXPORT float particle_density(struct particle_array *particles, int i,
                                  float h, float particle_mass,
                                  enum kernel_type type);
SPH_EXPORT Vector2 particle_pressure_gradient(struct particle_array *particles,
                                              int i, float h,
                                              float particle_mass,
                                              enum kernel_type kernel_type);

// Kernel functions
SPH_EXPORT float kernel_gaussian(float x, float h);
SPH_EXPORT float kernel_gaussian_derivative(float x, float h);
SPH_EXPORT float kernel_cubic(float x, float h);
SPH_EXPORT float kernel_cubic_derivative(float x, float h);
SPH_EXPORT float kernel_function(float x, float h, enum kernel_type type);
SPH_EXPORT float kernel_function_derivative(float x, float h,
                                            enum kernel_type type);

// Pressure computation
SPH_EXPORT float pressure_cole(float density, float rest_density,
                               float speed_of_sound, float adiabatic_index,
                               float background_pressure);
SPH_EXPORT float pressure_gas(float density, float rest_density,
                              float pressure_multiplier);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // SPH_H
