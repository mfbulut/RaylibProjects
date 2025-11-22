#include "raylib.h"
#include "raymath.h"

Font font56;
Font font48;
Font font32;
Font font18;

int score = 0;
int money = 20;
int hands = 3;

#include "utils.h"
#include "okey.h"
#include "game.h"

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1200, 540, "Okeyy");
    font56 = LoadFontEx("assets/Poppins-Medium.ttf", 56, 0, 1024);
    font48 = LoadFontEx("assets/Poppins-Medium.ttf", 48, 0, 1024);
    font32 = LoadFontEx("assets/Poppins-Medium.ttf", 32, 0, 1024);
    font18 = LoadFontEx("assets/Poppins-Medium.ttf", 18, 0, 1024);

    InitGame();

    while (!WindowShouldClose()) {
        UpdateGame();
        BeginDrawing();
        ClearBackground(BACKGROUND);
        DrawGame();
        EndDrawing();
    }

    UnloadFont(font56);
    UnloadFont(font48);
    UnloadFont(font32);
    UnloadFont(font18);
    CloseWindow();
}
