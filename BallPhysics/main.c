#include "raylib.h"
#include "raymath.h"

#define FULLSCREEN 0

#if FULLSCREEN
const int screenWidth = 1920;
const int screenHeight = 1080;
#else
const int screenWidth = 1280;
const int screenHeight = 720;
#endif
const float aspectRatio = (float)screenWidth / (float)screenHeight;
Camera2D camera;

#include "sdf.h"
#include "similation.h"

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Similation");
    SetTargetFPS(165);

    if(FULLSCREEN)
        ToggleBorderlessWindowed();

    camera.offset = (Vector2){ screenWidth / 2, screenHeight / 2 };
    camera.target = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = screenHeight / 2.0f;

    while (!WindowShouldClose()) {
        float deltaTime = Clamp(GetFrameTime(), 1.0f / 200.0f, 1.0 / 60.0f);
        Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

        const int subSteps = 32;
        float subStepTime = deltaTime / subSteps;
        for (int i = 0; i < subSteps; i++) {
            UpdateSimilation(subStepTime, mousePos);
        }

        if(IsKeyPressed(KEY_ONE)) CreateBall(0);
        if(IsKeyPressed(KEY_TWO)) CreateBall(1);
        if(IsKeyDown(KEY_THREE)) CreateBall(0);

        float cameraSpeed = 3.0f * deltaTime;
        if (IsKeyDown(KEY_W)) camera.target.y -= cameraSpeed;
        if (IsKeyDown(KEY_S)) camera.target.y += cameraSpeed;
        if (IsKeyDown(KEY_A)) camera.target.x -= cameraSpeed;
        if (IsKeyDown(KEY_D)) camera.target.x += cameraSpeed;

        float zoomSpeed = 200.0f;
        if (IsKeyDown(KEY_Q)) camera.zoom -= zoomSpeed * deltaTime;
        if (IsKeyDown(KEY_E)) camera.zoom += zoomSpeed * deltaTime;

        camera.zoom = Clamp(camera.zoom, 10.0f, 1000.0f);

        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera);
        DrawSimilation();
        EndMode2D();
        EndDrawing();
    }

    CloseWindow();
}
