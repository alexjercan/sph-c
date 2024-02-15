#include "raylib_extensions.h"
#include "sph.h"
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
float kernel_gaussian(float x, float h) {
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
float kernel_gaussian_derivative(float x, float h) {
    return (-2.0f * x) / (h * h) * kernel_gaussian(x, h);
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
float kernel_cubic(float x, float h) {
    float volume = M_PI * powf(h, 8) / 4.0f;
    float value = Max(0, h * h - x * x);
    return value * value * value / volume;
}

// Sebastian Lague's implementation of the derivative of the kernel function
float kernel_cubic_derivative(float x, float h) {
    if (x > h) {
        return 0.0f;
    }

    float f = h * h - x * x;
    float scale = -24.0 / (M_PI * powf(h, 8));
    return scale * x * f * f;
}

// Sebastian Lague's linear kernel function
float kernel_linear(float x, float h) {
    if (x >= h) {
        return 0.0f;
    }

    float volume = (M_PI * powf(h, 4)) / 6.0f;
    return (h - x) * (h - x) / volume;
}

// Sebastian Lague's linear kernel derivative
float kernel_linear_derivative(float x, float h) {
    if (x >= h) {
        return 0.0f;
    }

    float scale = 12.0f / (M_PI * powf(h, 4));
    return (h - x) * scale;
}

// Kernel wrapper function that selects the appropriate kernel function based on
// the kernel type
float kernel_function(float x, float h, enum kernel_type type) {
    switch (type) {
    case GAUSSIAN_KERNEL:
        return kernel_gaussian(x, h);
    case CUBIC_KERNEL:
        return kernel_cubic(x, h);
    case LINEAR_KERNEL:
        return kernel_linear(x, h);
    default:
        SPH_LOG_ERROR("Unknown kernel type %d", type);
        return 0.0f;
    }
}

// Kernel wrapper function that selects the appropriate kernel function based on
// the kernel type
float kernel_function_derivative(float x, float h, enum kernel_type type) {
    switch (type) {
    case GAUSSIAN_KERNEL:
        return kernel_gaussian_derivative(x, h);
    case CUBIC_KERNEL:
        return kernel_cubic_derivative(x, h);
    case LINEAR_KERNEL:
        return kernel_linear_derivative(x, h);
    default:
        SPH_LOG_ERROR("Unknown kernel type %d", type);
        return 0.0f;
    }

    return 0.0f;
}
