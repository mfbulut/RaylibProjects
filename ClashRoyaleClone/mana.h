float mana = 0.0f;

void UpdateMana() {
    mana += GetFrameTime();
    if (mana > 10.0f) mana = 10.1f;
}

void DrawMana() {
    int barWidth = 22;
    int barHeight = 10;
    int padding = 2;
    int startX = (screenWidth - (barWidth * 10 + padding * 9)) / 2;
    int yPosition = screenHeight - barHeight - 10;

    for (int i = 0; i < 10; i++) {
        Rectangle bar = { startX + i * (barWidth + padding), yPosition, barWidth, barHeight };
        if (mana >= i + 1) {
            DrawRectangleRec(bar, BLUE);
        } else {
            DrawRectangleLinesEx(bar, 1, GRAY);
            if(mana > i){
                bar.width *= (mana - ((int)mana));
                DrawRectangleRec(bar, BLUE);
            }
        }
    }
}