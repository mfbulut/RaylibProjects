#define BACKGROUND       (Color){ 90, 20 , 20, 255 }
#define PRIMARY          (Color){ 157, 109, 61, 255 }
#define PRIMARY_SHADOW   (Color){ 120, 80 , 30, 255 }

#define PIECE_COLOR          (Color){ 255, 240, 189, 255 }
#define PIECE_COLOR_SHADOW   (Color){ 216, 198, 142, 255 }

#define BUTTON_COLOR        (Color){ 28, 137, 35, 255 }
#define BUTTON_HOVER_COLOR  (Color){ 40, 150, 55, 255 }
#define BUTTON_CLICK_COLOR  (Color){ 15, 110, 25, 255 }
#define BUTTON_SHADOW_COLOR (Color){ 30, 90 , 10, 255 }
#define SHADOW_OFFSET (Vector2){ 0, 5 }

void DrawRectangleShadowed(Rectangle rect, float rounding, Color color, Color shadowColor, Vector2 shadowOffset) {
    DrawRectangleRounded((Rectangle){rect.x + shadowOffset.x, rect.y + shadowOffset.y, rect.width, rect.height}, rounding, 16, shadowColor);
    DrawRectangleRounded(rect, rounding, 16, color);
}

void DrawTextCentered(Font font, const char* text, Rectangle bounds, int fontSize, Color color) {
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 0);
    Vector2 textPosition = { bounds.x + (bounds.width / 2) - (textSize.x / 2),bounds.y + (bounds.height / 2) - (textSize.y / 2) };
    DrawTextEx(font, text, textPosition, fontSize, 0, color);
}

bool Button(Rectangle rect, const char* text) {
    bool isHovered = CheckCollisionPointRec(GetMousePosition(), rect);
    bool isPressed = isHovered && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    bool isClicked = isHovered && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);

    Color buttonColor =  isClicked ? BUTTON_CLICK_COLOR : (isHovered ? BUTTON_HOVER_COLOR : BUTTON_COLOR);

    if(isHovered){
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    } else {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if(isPressed) {
        Rectangle new = (Rectangle){rect.x, rect.y + 3, rect.width, rect.height};
        DrawRectangleShadowed(new, 0.25f, buttonColor, BUTTON_SHADOW_COLOR, (Vector2){ 0, 2 });
        if(rect.height > 60)
            DrawTextCentered(font56, text, new, 56, WHITE);
        else
            DrawTextCentered(font32, text, new, 32, WHITE);

    } else {
        DrawRectangleShadowed(rect, 0.25f, buttonColor, BUTTON_SHADOW_COLOR, SHADOW_OFFSET);
        if(rect.height > 60)
            DrawTextCentered(font56, text, rect, 56, WHITE);
        else
            DrawTextCentered(font32, text, rect, 32, WHITE);
    }

    return isClicked;
}


#define PROGRESS_DARK        (Color){ 82, 51, 19, 255 }
#define PROGRESS_LIGHT       (Color){ 111, 68, 26, 255 }
#define PROGRESS_FILL_DARK   (Color){ 46, 121, 26, 255 }
#define PROGRESS_FILL_LIGHT  (Color){ 28, 137, 35, 255 }

void DrawProgressBar(int score, int x, int y, int width, int height) {
    DrawRectangleRounded((Rectangle){ x, y + 3, width, height }, 1.0f, 16, PROGRESS_DARK);
    DrawRectangleRounded((Rectangle){ x, y, width, height }, 1.0f, 16, PROGRESS_LIGHT);

    int goal = 100;
    int pixels = (float)score / (float)goal * (float)width;
    DrawRectangleRounded((Rectangle){ x, y + 3, pixels, height }, 1.0f, 16, PROGRESS_FILL_DARK);
    DrawRectangleRounded((Rectangle){ x, y, pixels, height }, 1.0f, 16, PROGRESS_FILL_LIGHT);

    if(score > 7 && score < 100 - 7) {
        DrawRectangleRec((Rectangle){ x + pixels / 2, y + 3, pixels / 2, height }, PROGRESS_FILL_DARK);
        DrawRectangleRec((Rectangle){ x + pixels / 2, y, pixels / 2, height }, PROGRESS_FILL_LIGHT);
    }
}


void DrawUI() {
    DrawRectangleShadowed((Rectangle){ 10,  10,  250, 100 }, 0.2f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, 6  });
    DrawRectangleShadowed((Rectangle){ 270, 10,  560, 100 }, 0.2f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, 6  });
    DrawRectangleShadowed((Rectangle){ 840, 10,  350, 515 }, 0.1f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, 6  });

    DrawRectangleShadowed((Rectangle){ 10,  340, 820, 90  }, 0.1f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, -6 });
    DrawRectangleShadowed((Rectangle){ 10,  340 + 90, 820, 90  }, 0.0f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, -6 });
    DrawRectangleShadowed((Rectangle){ 10,  340 + 180, 820, 10 }, 0.0f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, -6 });

    DrawRectangleShadowed((Rectangle){ 750, 280, 80, 40  }, 0.3f, PRIMARY, PRIMARY_SHADOW, (Vector2){ 0, 6  });
    DrawTextCentered(font48, TextFormat("%d Puan", score), (Rectangle){10, 10, 250, 70}, 48, WHITE);
    DrawTextCentered(font32, TextFormat("%d$", money), (Rectangle){750, 280, 80, 40}, 32, WHITE);
    DrawProgressBar(score, 35, 75, 200, 15);

    for (int i = 0; i < hands; i++)
    {
        DrawRectangleShadowed((Rectangle){ 20 + i * 80, 123, 20, 30 }, 0.3f, PIECE_COLOR, PIECE_COLOR_SHADOW, (Vector2){ 0, 3  });
        DrawRectangleShadowed((Rectangle){ 20 + i * 80 + 25, 123, 20, 30 }, 0.3f, PIECE_COLOR, PIECE_COLOR_SHADOW, (Vector2){ 0, 3  });
        DrawRectangleShadowed((Rectangle){ 20 + i * 80 + 50, 123, 20, 30 }, 0.3f, PIECE_COLOR, PIECE_COLOR_SHADOW, (Vector2){ 0, 3  });
    }
}