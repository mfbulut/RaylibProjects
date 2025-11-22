void DrawBackground() {
    int width = 60;
    Color color40 = { 40, 40, 40, 255 };
    Color color80 = { 80, 80, 80, 255 };

    DrawRectangle(50 + 85 * 0 - width / 2, 0, width, 470, color40);
    DrawRectangle(50 + 85 * 1 - width / 2, 0, width, 470, color40);
    DrawRectangle(50 + 85 * 2 - width / 2, 0, width, 470, color40);
    DrawRectangle(0, 470, 270, 5, color80);
}