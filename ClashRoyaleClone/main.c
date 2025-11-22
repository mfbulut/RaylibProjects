#include "raylib.h"
#include "raymath.h"

const int screenWidth = 270;
const int screenHeight = 600;

#include "background.h"
#include "mana.h"
#include "heroes.h"
#include "cards.h"

int main()
{
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Game");


    while (!WindowShouldClose())
    {
        // Logic
        UpdateMana();
        UpdateCards();
        UpdateHeroes();

        // Drawing
        BeginDrawing();
        ClearBackground(BLACK);
        DrawBackground();
        DrawHeroes();
        DrawMana();
        DrawCards();
        EndDrawing();
    }

    CloseWindow();

}
