#include "raylib.h"
#include "raymath.h"

#define FPHYSICS_IMPLEMENTATION
#include "fphysics.h"
#include "joint.h"

const int width = 1280, height = 720;

#include "game.h"

int main(void) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, "Physics Game");

    InitGame();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateGame(deltaTime);

        BeginDrawing();
        ClearBackground(BLACK);

        DrawGame();

        EndDrawing();
    }

    CloseWindow();
    return 0;
}