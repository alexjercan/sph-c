#include "raylib.h"
#include "raymath.h"
#include "sph.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define X_RANGE_START -4.0f
#define X_RANGE_END 4.0f
#define Y_RANGE_START 0.0f
#define Y_RANGE_END 3.0f
#define STEP 0.1f

void PlotCubicKernel(float h) {
    float width = SCREEN_WIDTH * 1.0f;
    float height = SCREEN_HEIGHT * 1.0f;
    float x_start = X_RANGE_START;
    float x_end = X_RANGE_END;
    float y_start = Y_RANGE_START;
    float y_end = Y_RANGE_END;
    float step = STEP;

    // Draw the x and y axes
    DrawLineEx((Vector2){width / 2, 0}, (Vector2){width / 2, height}, 1, RED);
    DrawLineEx((Vector2){0, height * 0.9f}, (Vector2){width, height * 0.9f}, 1,
               RED);

    for (float x = x_start; x <= x_end; x += step * 5) {
        float x_screen = (x - x_start) / (x_end - x_start) * width;
        float y_screen = height * 0.9f;
        DrawText(TextFormat("%.1f", x), x_screen - 10, y_screen + 10, 10, WHITE);
        DrawCircleV((Vector2){x_screen, y_screen}, 2, RED);

    }

    for (float y = y_start; y <= y_end; y += step * 5) {
        float x_screen = (width / 2);
        float y_screen = (height * 0.9f - (y - y_start) / (y_end - y_start) * height * 0.9f);
        DrawText(TextFormat("%.1f", y), x_screen + 5, y_screen - 10, 10, WHITE);
        DrawCircleV((Vector2){x_screen, y_screen}, 2, RED);
    }

    // Draw two vertical lines at -h and h
    DrawLineEx(
        (Vector2){(-h - x_start) / (x_end - x_start) * width, 0},
        (Vector2){(-h - x_start) / (x_end - x_start) * width,
                  height},
        1, WHITE);
    DrawLineEx(
        (Vector2){(h - x_start) / (x_end - x_start) * width, 0},
        (Vector2){(h - x_start) / (x_end - x_start) * width,
                  height},
        1, WHITE);

    // Draw the Gaussian kernel function
    Vector2 prev_point = (Vector2){0, 0};
    for (float x = x_start; x <= x_end; x += step) {
        float y = kernel_linear(x, h);
        float x_screen = (x - x_start) / (x_end - x_start) * width;
        float y_screen = (height * 0.9f - (y - y_start) / (y_end - y_start) * height * 0.9f);

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
        float y = kernel_linear_derivative(x, h);
        float x_screen = (x - x_start) / (x_end - x_start) * width;
        float y_screen = (height * 0.9f - (y - y_start) / (y_end - y_start) * height * 0.9f);

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
        h = Clamp(h, 0.1f, (X_RANGE_END - X_RANGE_START) / 2.0f);

        PlotCubicKernel(h);

        DrawText(TextFormat("h: %f m (scroll to change)", h), 10, 10, 20, WHITE);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
