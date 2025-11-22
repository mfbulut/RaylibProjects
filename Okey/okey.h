typedef struct {
    int number;
    int color;
    Vector2 currentPos;
    Vector2 targetPos;
    float animProgress;
} Piece;

typedef struct {
    bool isDragging;
    int draggedPieceIndex;
    Vector2 dragOffset;
    Vector2 originalPos;
} DragState;

#define BOARD_SIZE 32
#define ANIM_SPEED 5.0f

Piece hand[BOARD_SIZE] = {0};
DragState dragState = {0};

Vector2 GetPiecePosition(int index) {
    return (Vector2){
        22 + (index % 16) * 50,
        355 + (index / 16) * 90
    };
}

Rectangle GetPieceRect(int index) {
    Vector2 pos = GetPiecePosition(index);
    return (Rectangle){ pos.x - 3.0f, pos.y, 48 + 3.0f, 72 };
}

void UpdateHand() {
    float deltaTime = GetFrameTime();
    Vector2 mousePos = GetMousePosition();

    // Update animations
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (hand[i].number > 0 && hand[i].animProgress < 1.0f) {
            hand[i].animProgress += deltaTime * ANIM_SPEED;
            if (hand[i].animProgress > 1.0f) hand[i].animProgress = 1.0f;
            hand[i].currentPos = Vector2Lerp(hand[i].currentPos, hand[i].targetPos, hand[i].animProgress);
        }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !dragState.isDragging) {
        for (int i = BOARD_SIZE-1; i >= 0; i--) {
            if (hand[i].number > 0) {
                Rectangle pieceRect = GetPieceRect(i);
                if (CheckCollisionPointRec(mousePos, pieceRect)) {
                    dragState.isDragging = true;
                    dragState.draggedPieceIndex = i;
                    dragState.originalPos = hand[i].currentPos;
                    dragState.dragOffset = Vector2Subtract(dragState.originalPos, mousePos);
                    break;
                }
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && dragState.isDragging) {
        for (int i = 0; i < BOARD_SIZE; i++) {
            Rectangle dropRect = GetPieceRect(i);
            Vector2 dragPos = Vector2Add(Vector2Add(mousePos, dragState.dragOffset), (Vector2){48.0f / 2.0f, 72.0f / 2.0f});

            if (CheckCollisionPointRec(dragPos, dropRect)) {

                Vector2 pos1 = hand[dragState.draggedPieceIndex].currentPos;
                Vector2 pos2 = hand[i].currentPos;


                Piece temp = hand[dragState.draggedPieceIndex];
                hand[dragState.draggedPieceIndex] = hand[i];
                hand[i] = temp;


                hand[i].currentPos = Vector2Add(mousePos, dragState.dragOffset);
                hand[i].targetPos = GetPiecePosition(i);
                hand[i].animProgress = 0.0f;

                if (hand[dragState.draggedPieceIndex].number > 0) {
                    hand[dragState.draggedPieceIndex].currentPos = pos2;
                    hand[dragState.draggedPieceIndex].targetPos = GetPiecePosition(dragState.draggedPieceIndex);
                    hand[dragState.draggedPieceIndex].animProgress = 0.0f;
                }
                break;
            }
        }
        dragState.isDragging = false;
    }
}


void DrawPiece(Piece* piece) {
    Vector2 pos = piece->currentPos;
    int number = piece->number;
    int color = piece->color;

    Color colors[4] = { (Color){ 12, 12, 12, 255 }, RED, (Color){ 160, 140, 0, 255 }, BLUE};
    Color pieceColor = colors[color];

    DrawRectangleShadowed((Rectangle){ pos.x, pos.y, 48, 72 }, 0.3f, PIECE_COLOR, PIECE_COLOR_SHADOW, (Vector2){ 0, -4 });
    if(number != 14) {
        DrawTextCentered(font32, TextFormat("%d", number), (Rectangle){ pos.x, pos.y, 48, 40 }, 32, pieceColor);
    } else{
        DrawTextCentered(font56, "*", (Rectangle){ pos.x, pos.y, 48, 56 }, 56, pieceColor);
    }

    // DrawCircle(pos.x + 24, pos.y + 50, 8, PIECE_COLOR_SHADOW);
    // DrawCircleGradient(pos.x + 24, pos.y + 50, 8, (Color) { 0, 0, 0, 16}, (Color) { 255, 255, 255, 32});
}

void DrawHand() {
    Vector2 mousePos = GetMousePosition();

    for (int i = 0; i < BOARD_SIZE; i++) {
        if (i != dragState.draggedPieceIndex || !dragState.isDragging) {
            if (hand[i].number > 0) {
                DrawPiece(&hand[i]);
            }
        }
    }

    if (dragState.isDragging) {
        Vector2 dragPos = Vector2Add(mousePos, dragState.dragOffset);
        hand[dragState.draggedPieceIndex].currentPos = dragPos;
        hand[dragState.draggedPieceIndex].animProgress = 0.0f;
        DrawPiece(&hand[dragState.draggedPieceIndex]);
    }
}

