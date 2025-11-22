typedef enum {
    BASIC
} HeroType;

typedef struct {
    Vector2 pos;
    int health;
    float timer;
    HeroType type;
} Hero;

Hero heroes[256];
int heroCount = 0;

void AddHero(int lane, HeroType type) {
    if (heroCount < 256) {
        Hero newHero;
        newHero.health = 100;
        newHero.pos = (Vector2){ 50 + 85 * lane, 450 };
        newHero.timer = 0.0f;
        newHero.type = type;

        heroes[heroCount++] = newHero;
    }
}

void UpdateHeroes() {
    for (int i = 0; i < heroCount; i++) {
        heroes[i].pos.y -= 0.2;
        heroes[i].timer += GetFrameTime();
    }
}

void DrawHeroes() {
    for (int i = 0; i < heroCount; i++) {
        switch (heroes[i].type) {
            case BASIC:
                DrawCircle(heroes[i].pos.x, heroes[i].pos.y, 10, RED);
                break;
        }

        int healthBarWidth = 18;
        int healthBarHeight = 3;
        int healthBarX = heroes[i].pos.x - healthBarWidth / 2;
        int healthBarY = heroes[i].pos.y - healthBarHeight - 13;
        DrawRectangle(healthBarX, healthBarY, healthBarWidth, healthBarHeight, BLACK);
        int healthWidth = (heroes[i].health * healthBarWidth) / 100;
        DrawRectangle(healthBarX, healthBarY, healthWidth, healthBarHeight, GREEN);
    }
}