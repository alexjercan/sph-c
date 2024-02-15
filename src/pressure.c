#include "sph.h"
#include <math.h>

// Computes the pressure at a given point using the Cole equation
//
// The pressure is computed using the Cole equation, which is a more accurate
// approximation of the pressure in a fluid. The equation is given by:
//
//    P = B * (pow(density / rest_density, adiabatic_index) - 1)
//
// Where:
// - B is a constant that depends on the rest density, the speed of sound and
//  the adiabatic index.
// - density is the density at the point.
//
// Returns the pressure based on the density (in Pascals).
float pressure_cole(float density, float rest_density,
                            float speed_of_sound, float adiabatic_index,
                            float background_pressure) {
    float B = rest_density * speed_of_sound * speed_of_sound / adiabatic_index;
    float x = powf(density / rest_density, adiabatic_index) - 1.0f;

    return B * x + background_pressure;
}

// Computes the pressure at a given point using the gas approximation
//
// The pressure is computed using the gas approximation, which is a simple
// approximation of the pressure in a fluid. The equation is given by:
//
//   P = (density - rest_density) * pressure_multiplier
//
// Where:
// - density is the density at the point.
//
// Returns the pressure based on the density (in Pascals).
float pressure_gas(float density, float rest_density,
                           float pressure_multiplier) {
    float density_error = density - rest_density;
    float pressure = density_error * pressure_multiplier;
    return pressure;
}
