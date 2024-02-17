#include "raylib.h"
#include "raymath.h"
#include "sph.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

void PlotCubicKernel(float h) {
    float width = SCREEN_WIDTH * 1.0f;
    float height = SCREEN_HEIGHT * 1.0f;
    float x_start = 0.0f;
    float x_end = 4.0f;
    float y_start = -2.0f;
    float y_end = 2.0f;
    float step = 0.025f;

    // Draw the x and y axes
    DrawLineEx((Vector2){width * 0.1f, 0}, (Vector2){width * 0.1f, height}, 1, RED);
    DrawLineEx((Vector2){0, height * 0.5f}, (Vector2){width, height * 0.5f}, 1,
               RED);

    // Draw the ticks and labels for x axis
    for (float x = x_start; x <= x_end; x += 0.5f) {
        float x_screen = ((x - x_start) / (x_end - x_start) * width) + (width * 0.1f);
        float y_screen = height * 0.5f;
        DrawText(TextFormat("%.1f", x), x_screen - 10, y_screen + 10, 10, WHITE);
        DrawCircleV((Vector2){x_screen, y_screen}, 2, RED);

    }

    // Draw the ticks and labels for y axis
    for (float y = y_start; y <= y_end; y += 0.5f) {
        float x_screen = width * 0.1f;
        float y_screen = height - ((y - y_start) / (y_end - y_start) * height);
        DrawText(TextFormat("%.1f", y), x_screen + 5, y_screen - 10, 10, WHITE);
        DrawCircleV((Vector2){x_screen, y_screen}, 2, RED);
    }

    // Draw vertical lines at h
    DrawLineEx(
        (Vector2){(h - x_start) / (x_end - x_start) * width + width * 0.1f, 0},
        (Vector2){(h - x_start) / (x_end - x_start) * width + width * 0.1f,
                  height},
        1, WHITE);

    // Draw the Gaussian kernel function
    Vector2 prev_point = (Vector2){0, 0};
    for (float x = x_start; x <= x_end; x += step) {
        float y = kernel_gaussian(x, h);
        float x_screen = ((x - x_start) / (x_end - x_start) * width) + (width * 0.1f);
        float y_screen = height - ((y - y_start) / (y_end - y_start) * height);

        DrawCircleV((Vector2){x_screen, y_screen}, 2, BLUE);
        if (x != x_start) {
            DrawLineEx(prev_point, (Vector2){x_screen, y_screen}, 1, BLUE);
        }
        prev_point.x = x_screen;
        prev_point.y = y_screen;
    }

    // Draw the derivative of the Gaussian kernel function
    prev_point = (Vector2){0, 0};
    for (float x = x_start; x <= x_end; x += step) {
        float y = kernel_gaussian_derivative(x, h);
        float x_screen = ((x - x_start) / (x_end - x_start) * width) + (width * 0.1f);
        float y_screen = height - ((y - y_start) / (y_end - y_start) * height);

        DrawCircleV((Vector2){x_screen, y_screen}, 2, GREEN);
        if (x != x_start) {
            DrawLineEx(prev_point, (Vector2){x_screen, y_screen}, 1, GREEN);
        }
        prev_point.x = x_screen;
        prev_point.y = y_screen;
    }
}

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cubic Kernel Function");

    SetTargetFPS(60);
    float h = 1.0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        h += GetMouseWheelMove() * 0.1f;
        h = Clamp(h, 0.1f, 3.5f);

        PlotCubicKernel(h);

        DrawText(TextFormat("h: %f m (scroll to change)", h), 10, 10, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
