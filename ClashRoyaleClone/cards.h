int cards[12] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
int manaCosts[12] = { 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6 };

int selectedCard = -1;
int draggingCard = -1;
Vector2 draggingCardOffset = {0, 0};
bool justSelected = false;

void ShiftCards(int selectedCard) {
    for (int i = selectedCard; i < 11; i++) {
        cards[i] = cards[i + 1];
    }
    cards[11] = selectedCard + 1;
}

void SwapCards(int first, int second) {
    int temp = cards[first];
    cards[first] = cards[second];
    cards[second] = temp;
}

void UpdateCards() {
    int cardWidth = 50;
    int cardHeight = 75;
    int spacing = 10;
    int startX = 20;
    int startY = 495;

    Vector2 mousePosition = GetMousePosition();

    // Handle card selection with a tap
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        justSelected = false;
        for (int i = 0; i < 4; i++) {
            Rectangle rec = {startX + i * (cardWidth + spacing), startY, cardWidth, cardHeight};
            if (CheckCollisionPointRec(mousePosition, rec)) {
                if(selectedCard == i){
                    justSelected = false;
                } else {
                    justSelected = true;
                }

                selectedCard = i;
                break;
            }
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && !justSelected) {
        for (int i = 0; i < 4; i++) {
            Rectangle rec = {startX + i * (cardWidth + spacing), startY, cardWidth, cardHeight};
            if (CheckCollisionPointRec(mousePosition, rec)) {
                if (selectedCard == i) {
                    selectedCard = -1; // Unselect if already selected
                }
                break;
            }
        }
    }

    // Handle dragging if the card is selected
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) ) {
        for (int i = 0; i < 4; i++) {
            Rectangle rec = {startX + i * (cardWidth + spacing), startY - 10, cardWidth, cardHeight + 20};
            if (CheckCollisionPointRec(mousePosition, rec)) {
                if (draggingCard == -1) {
                    draggingCard = i;
                    draggingCardOffset.x = mousePosition.x - (startX + draggingCard * (cardWidth + spacing));
                    draggingCardOffset.y = mousePosition.y - startY;
                    if(!justSelected) {
                        draggingCardOffset.y += 10;
                    }
                }
            }
        }
    } else {
        if(draggingCard > -1 && (float)manaCosts[cards[draggingCard]] <= mana && GetMouseY() < 460) {
            mana -= manaCosts[cards[draggingCard]];
            ShiftCards(draggingCard);
            selectedCard = -1;
            draggingCard = -1;

            float mouseX = GetMouseX();
            float dist1 = fabs(mouseX - 50);
            float dist2 = fabs(mouseX - 135);
            float dist3 = fabs(mouseX - 220);

            if (dist1 < dist2 && dist1 < dist3)
            {
                AddHero(0, BASIC);
            }
            else if (dist2 < dist1 && dist2 < dist3)
            {
                AddHero(1, BASIC);
            }
            else
            {
                AddHero(2, BASIC);
            }

        }
        draggingCard = -1;
    }

    // Handle swapping if a card is being dragged
    if (draggingCard != -1) {
        for (int i = 0; i < 4; i++) {
            Rectangle rec = {startX + i * (cardWidth + spacing), startY, cardWidth, cardHeight};
            if (CheckCollisionPointRec(mousePosition, rec) && i != draggingCard) {
                SwapCards(draggingCard, i);
                selectedCard = -1;
                draggingCard = i;
                break;
            }
        }
    }
}

void DrawCards() {
    int cardWidth = 50;
    int cardHeight = 75;
    int spacing = 10;
    int startX = 20;
    int startY = 495;

    for (int i = 0; i < 4; i++) {
        Rectangle rec = {startX + i * (cardWidth + spacing), startY, cardWidth, cardHeight};

        // Draw selected card slightly higher to indicate selection
        if (i == selectedCard && draggingCard == -1) {
            rec.y -= 10;
        }

        // Draw dragging card following the mouse
        if (i == draggingCard) {
            rec.x = GetMouseX() - draggingCardOffset.x;
            rec.y = GetMouseY() - draggingCardOffset.y;

            if(GetMouseY() < 460) {
                DrawCircleV(GetMousePosition(), 8, LIGHTGRAY);
                continue;
            }
        }

        DrawRectangleRounded(rec, 0.1f, 4, DARKGRAY);
        DrawText(TextFormat("%d", cards[i]), rec.x + cardWidth / 2 - 6, rec.y + cardHeight / 2 - 10, 20, WHITE);
        DrawText(TextFormat("%d", manaCosts[cards[i]]), rec.x + cardWidth - 15, rec.y + 2, 20, (Color){255, 40, 255, 255});
    }
}
