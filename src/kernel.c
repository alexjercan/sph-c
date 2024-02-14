#include "sph.h"
#include "raylib_extensions.h"
#include <math.h>

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
    return (1.0f / (h * sqrtf(M_PI))) * expf(-1.0f * (x * x) / (h * h));
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

// Sebastian Lague's implementation of the kernel function
//
// The kernel function is used to compute the influence of a particle on another
// particle. It is defined as:
//
// W(x, h) = (h^2 - x^2)^3 * (PI * h^8 / 4) if x < h
//         = 0 otherwise
//
// Where:
// - x is the distance between the two particles (in meters)
// - h is the smoothing length (in meters)
//
// Returns the influence of a particle on another particle (in 1/m)
float cubic_kernel_function(float x, float h) {
    float volume = M_PI * powf(h, 8) / 4.0f;
    float value = Max(0, h * h - x * x);
    return value * value * value / volume;
}

// Sebastian Lague's implementation of the derivative of the kernel function
float cubic_kernel_function_derivative(float x, float h) {
    if (x > h) {
        return 0.0f;
    }

    float f = h * h - x * x;
    float scale = -24.0 / (M_PI * powf(h, 8));
    return scale * x * f * f;
}
