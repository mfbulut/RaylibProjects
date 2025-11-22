#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 540
#define GRAVITY 1160.0f
#define JUMP_FORCE -420.0f
#define PIPE_SPEED 225.0f
#define PIPE_WIDTH 90
#define PIPE_GAP 150
#define BIRD_SIZE 40
#define MAX_PIPES 512
#define MAX_ROTATION 30.0f
#define ROTATION_SPEED 170.0f
#define RESPAWN_TIME 8.0f
#define TRAIL_LENGTH 32

typedef struct {
    Vector2 position;
    float velocity;
    bool isAlive;
    double respawnTime;
    float rotation;
    bool respawnProtection;
    Vector2 trail[TRAIL_LENGTH];
    int trailIndex;
} Bird;

typedef struct {
    Rectangle top;
    Rectangle bottom;
    bool counted;
} Pipe;

Bird birds[2];
Pipe pipes[MAX_PIPES];
int pipeCount = 0;
int score = 0;
float gameSpeed = 1.0f;
bool gameStarted = false;
double gameTime = 0.0;
Font customFont;
Texture2D redBirdTexture;
Texture2D happyBirdTexture;
Sound coin;
bool gameOver = false;

#define MAX_TOUCH_POINTS 10

typedef struct {
    int touchId;
    bool active;
    float lastJumpTime;
} TouchPoint;

TouchPoint touchPoints[MAX_TOUCH_POINTS] = {0};

// Keyboard timing for jump prevention
float lastSpaceTime = -1.0f;
float lastShiftTime = -1.0f;
float lastCtrlTime = -1.0f;

#define MAX_BUILDINGS 5
#define NUM_LAYERS 3
#define MAX_CLOUDS 7

typedef struct {
    Rectangle rect;
    Color color;
    float speed;
} Building;

Building buildings[NUM_LAYERS][MAX_BUILDINGS];

typedef struct {
    Vector2 position;
    float size;
    float speed;
} Cloud;

Cloud clouds[MAX_CLOUDS];

void InitParallaxBackground(void) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        clouds[i] = (Cloud){
            {GetRandomValue(0, SCREEN_WIDTH), GetRandomValue(0, SCREEN_HEIGHT / 3)},
            GetRandomValue(50, 120),
            10.0f + GetRandomValue(0, 10)
        };
    }

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            float height = GetRandomValue(90, 280);
            float width = GetRandomValue(100, 150);
            float x = i * (SCREEN_WIDTH / MAX_BUILDINGS) + GetRandomValue(-20, 20);
            buildings[layer][i] = (Building){
                {x, SCREEN_HEIGHT - height, width, height},
                (Color){
                    10 + 8 * layer + GetRandomValue(0, 12),
                    45 + 10 * layer + GetRandomValue(-10, 8),
                    70 + 10 * layer + GetRandomValue(-10, 8),
                    255
                },
                (layer + 1) * 30.0f
            };
        }
    }
}

void UpdateParallaxBackground(float deltaTime) {
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            buildings[layer][i].rect.x -= buildings[layer][i].speed * deltaTime * gameSpeed;
            if (buildings[layer][i].rect.x + buildings[layer][i].rect.width < 0) {
                buildings[layer][i].rect.x = 1200;
            }
        }
    }
}

void DrawCloud(Vector2 position, float size, Color color) {
    DrawRectangleRounded((Rectangle){position.x - size / 2, position.y, size, size / 3}, 1, 32, color);
}

void DrawParallaxBackground(void) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        DrawCloud(clouds[i].position, clouds[i].size, (Color){100, 145, 170, 255});
    }

    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        for (int i = 0; i < MAX_BUILDINGS; i++) {
            DrawRectangleRec(buildings[layer][i].rect, buildings[layer][i].color);
        }
    }
}


void InitGame(void) {
    for (int i = 0; i < 2; i++) {
        birds[i] = (Bird){{(i + 1) * SCREEN_WIDTH / 3, SCREEN_HEIGHT / 2 - 100}, 0, true, 0, 0, false};
        for (int j = 0; j < TRAIL_LENGTH; j++) {
            birds[i].trail[j] = birds[i].position;
        }
        birds[i].trailIndex = 0;
    }
    pipeCount = 0;
    score = 0;
    gameSpeed = 1.0f;
    gameStarted = false;
    gameTime = 0.0;
    customFont = LoadFontEx("assets/PAPYRUS.TTF", 160, 0, 250);
    redBirdTexture = LoadTexture("assets/p2.png");
    happyBirdTexture = LoadTexture("assets/p1.png");
    coin = LoadSound("assets/sound.qoa");
    SetSoundVolume(coin, 0.5f);

    // Reset keyboard timing
    lastSpaceTime = -1.0f;
    lastShiftTime = -1.0f;
    lastCtrlTime = -1.0f;
}

void AddPipe(void) {
    if (pipeCount < MAX_PIPES) {
        float gapY = GetRandomValue(PIPE_GAP, SCREEN_HEIGHT - PIPE_GAP);
        pipes[pipeCount].top = (Rectangle){SCREEN_WIDTH + 150, -100, PIPE_WIDTH, gapY - PIPE_GAP / 2 + 100};
        pipes[pipeCount].bottom = (Rectangle){SCREEN_WIDTH + 150, gapY + PIPE_GAP / 2, PIPE_WIDTH, SCREEN_HEIGHT - (gapY + PIPE_GAP / 2) + 100};
        pipes[pipeCount].counted = false;
        pipeCount++;
    }
}

bool CheckBirdPipeCollision(Bird bird, Pipe pipe) {
    if (bird.respawnProtection) return false;
    int eays = 3;
    Rectangle birdRect = {bird.position.x - BIRD_SIZE / 2 + eays, bird.position.y - BIRD_SIZE / 2 + eays, BIRD_SIZE - eays * 2, BIRD_SIZE - eays * 2};
    return CheckCollisionRecs(birdRect, pipe.top) || CheckCollisionRecs(birdRect, pipe.bottom);
}


void HandleBirdDeath(int birdIndex) {
    birds[birdIndex].isAlive = false;
    birds[birdIndex].respawnTime = gameTime + RESPAWN_TIME;
    birds[birdIndex].rotation = -10;
    birds[birdIndex].respawnProtection = false;

    for (int i = 0; i < TRAIL_LENGTH; i++) {
        birds[birdIndex].trail[i] = (Vector2){-100, -100};
    }
}

void UpdateBird(Bird *bird, float deltaTime) {
    bird->trailIndex = (bird->trailIndex + 1) % TRAIL_LENGTH;
    bird->trail[bird->trailIndex] = bird->position;

    bird->velocity += GRAVITY * gameSpeed * deltaTime;
    bird->position.y += bird->velocity * deltaTime;

    bird->rotation += ROTATION_SPEED * gameSpeed * deltaTime;
    if (bird->rotation > MAX_ROTATION) bird->rotation = MAX_ROTATION;

    if (bird->position.y <= -75 || bird->position.y + BIRD_SIZE >= SCREEN_HEIGHT + 75) {
        if(bird == &birds[0])
            HandleBirdDeath(0);
        else
            HandleBirdDeath(1);
    }
}

void UpdateBirds(float deltaTime) {
    bool anyBirdAlive = false;
    for (int i = 0; i < 2; i++) {
        if (birds[i].isAlive) {
            anyBirdAlive = true;
            UpdateBird(&birds[i], deltaTime);
        } else if (gameTime >= birds[i].respawnTime) {
            birds[i].isAlive = true;
            birds[i].respawnProtection = true;
            birds[i].position.y = SCREEN_HEIGHT / 2;
            birds[i].velocity = 0;
            birds[i].rotation = -10;
        }

        if (birds[i].isAlive && birds[i].respawnProtection && gameTime >= birds[i].respawnTime + 2.0f) {
            birds[i].respawnProtection = false;
        }
    }

    if (!anyBirdAlive) {
        InitGame();
    }
}

void UpdatePipes(float deltaTime) {
    for (int i = 0; i < pipeCount; i++) {
        pipes[i].top.x -= PIPE_SPEED * gameSpeed * deltaTime;
        pipes[i].bottom.x -= PIPE_SPEED * gameSpeed * deltaTime;

        for (int j = 0; j < 2; j++) {
            if (birds[j].isAlive && CheckBirdPipeCollision(birds[j], pipes[i])) {
                HandleBirdDeath(j);
            }
        }

        if (!pipes[i].counted ) {
            if(birds[1].isAlive && pipes[i].top.x - PIPE_WIDTH / 2 < SCREEN_WIDTH / 2 + 100) {
                score++;
                SetSoundPitch(coin, 0.8f + (float)GetRandomValue(0, 200) / 1000.0f);
                PlaySound(coin);
                pipes[i].counted = true;
            } else if(pipes[i].top.x - PIPE_WIDTH / 2 < SCREEN_WIDTH / 3 - 100){
                score++;
                SetSoundPitch(coin, 0.8f + (float)GetRandomValue(0, 200) / 1000.0f);
                PlaySound(coin);
                pipes[i].counted = true;
            }
        }
    }

    while (pipeCount > 0 && pipes[0].top.x + PIPE_WIDTH < 0) {
        for (int i = 1; i < pipeCount; i++) {
            pipes[i - 1] = pipes[i];
        }
        pipeCount--;
    }

    if (pipeCount == 0 || pipes[pipeCount - 1].top.x < SCREEN_WIDTH - 300) {
        AddPipe();
    }
}

void UpdateGame(float deltaTime) {
    if (!gameStarted) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || GetTouchPointCount() > 0 ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT) ||
            IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL)) {
            gameStarted = true;
            InitParallaxBackground();
        }
        return;
    }

    if (gameOver) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) || GetTouchPointCount() > 0 ||
            IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT) ||
            IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL)) {
            InitGame();
            gameOver = false;
            InitParallaxBackground();
            gameStarted = true;
        }
        return;
    }

    gameTime += deltaTime;

    // Handle keyboard input with timing prevention
    if (IsKeyPressed(KEY_SPACE) && gameTime - lastSpaceTime > 0.2f && birds[0].isAlive) {
        birds[0].velocity = JUMP_FORCE * gameSpeed;
        birds[0].rotation = -MAX_ROTATION;
        lastSpaceTime = gameTime;
    }

    if ((IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) &&
        gameTime - lastShiftTime > 0.2f && birds[1].isAlive) {
        birds[1].velocity = JUMP_FORCE * gameSpeed;
        birds[1].rotation = -MAX_ROTATION;
        lastShiftTime = gameTime;
    }

    if ((IsKeyPressed(KEY_LEFT_CONTROL) || IsKeyPressed(KEY_RIGHT_CONTROL)) &&
        gameTime - lastCtrlTime > 0.2f && birds[1].isAlive) {
        birds[1].velocity = JUMP_FORCE * gameSpeed;
        birds[1].rotation = -MAX_ROTATION;
        lastCtrlTime = gameTime;
    }

    // Handle touch input
    int touchCount = GetTouchPointCount();
    for (int i = 0; i < touchCount; i++) {
        Vector2 touchPos = GetTouchPosition(i);
        int touchId = GetTouchPointId(i);

        int touchIndex = -1;
        for (int j = 0; j < MAX_TOUCH_POINTS; j++) {
            if (touchPoints[j].touchId == touchId) {
                touchIndex = j;
                break;
            } else if (!touchPoints[j].active) {
                touchIndex = j;
                touchPoints[j].touchId = touchId;
                touchPoints[j].active = true;
                touchPoints[j].lastJumpTime = -1.0f;
                break;
            }
        }

        if (touchIndex != -1 && gameTime - touchPoints[touchIndex].lastJumpTime > 0.2f) {
            if (touchPos.x < SCREEN_WIDTH / 2 && birds[0].isAlive) {
                birds[0].velocity = JUMP_FORCE * gameSpeed;
                birds[0].rotation = -MAX_ROTATION;
                touchPoints[touchIndex].lastJumpTime = gameTime;
            } else if (birds[1].isAlive) {
                birds[1].velocity = JUMP_FORCE * gameSpeed;
                birds[1].rotation = -MAX_ROTATION;
                touchPoints[touchIndex].lastJumpTime = gameTime;
            }
        }
    }

    for (int i = 0; i < MAX_TOUCH_POINTS; i++) {
        bool found = false;
        for (int j = 0; j < touchCount; j++) {
            if (touchPoints[i].touchId == GetTouchPointId(j)) {
                found = true;
                break;
            }
        }
        if (!found) {
            touchPoints[i].active = false;
        }
    }

    // Handle mouse input
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        if (mousePos.x < SCREEN_WIDTH / 2 && birds[0].isAlive) {
            birds[0].velocity = JUMP_FORCE * gameSpeed;
            birds[0].rotation = -MAX_ROTATION;
        } else if (birds[1].isAlive) {
            birds[1].velocity = JUMP_FORCE * gameSpeed;
            birds[1].rotation = -MAX_ROTATION;
        }
    }

    UpdateParallaxBackground(deltaTime);
    UpdateBirds(deltaTime);
    UpdatePipes(deltaTime);

    gameSpeed += 0.01f * deltaTime;

    if (!birds[0].isAlive && !birds[1].isAlive) {
        gameOver = true;
    }
}

void DrawBird(Bird bird, Texture2D texture) {
    for (int i = 0; i < TRAIL_LENGTH; i++) {
        int index = (bird.trailIndex - i + TRAIL_LENGTH) % TRAIL_LENGTH;
        float alpha = (float)(TRAIL_LENGTH - i) / TRAIL_LENGTH;
        Color trailColor = {255, 255, 255, (unsigned char)(alpha * 128)};
        DrawCircle(bird.trail[index].x - i, bird.trail[index].y, BIRD_SIZE / 5 * alpha, trailColor);
    }

    Vector2 origin = {BIRD_SIZE / 2, BIRD_SIZE / 2};

    DrawTexturePro(texture,
                   (Rectangle){0, 0, texture.width, texture.height},
                   (Rectangle){bird.position.x, bird.position.y, BIRD_SIZE, BIRD_SIZE},
                   origin,
                   bird.rotation,
                   bird.respawnProtection ? (Color){255,255,255,128} : WHITE);
}

void DrawGame(void) {
    BeginDrawing();
    ClearBackground((Color){0,90,140,255});

    if (!gameStarted) {
        DrawTextEx(customFont, "Freaky Birds", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Freaky Birds", 162, 0).x / 2, SCREEN_HEIGHT / 6}, 162, 0, BLACK);
        DrawTextEx(customFont, "Freaky Birds", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Freaky Birds", 160, 0).x / 2, SCREEN_HEIGHT / 6}, 160, 0, WHITE);
        DrawTextEx(customFont, "Click anywhere to start", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Click anywhere to start", 80,  0).x / 2, SCREEN_HEIGHT / 2 + 50}, 80, 0, WHITE);
        DrawTextEx(customFont, "Player 1: SPACE | Player 2: SHIFT/CTRL", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Player 1: SPACE | Player 2: SHIFT/CTRL", 40,  0).x / 2, SCREEN_HEIGHT / 2 + 150}, 40, 0, WHITE);
        DrawTextEx(customFont, "Made by Furkan", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Made by Furkan", 40,  0).x / 2, SCREEN_HEIGHT - 50}, 40, 0, WHITE);
    } else if (gameOver) {
        DrawParallaxBackground();

        for (int i = 0; i < pipeCount; i++) {
            Rectangle topPipe = pipes[i].top;
            Rectangle bottomPipe = pipes[i].bottom;
            topPipe.width += 5;
            bottomPipe.width += 5;
            DrawRectangleRounded(topPipe, 0.1f, 16, (Color){40,255,40,255});
            DrawRectangleRounded(bottomPipe, 0.1f, 16, (Color){40,255,40,255});
        }

        for (int i = 0; i < 2; i++) {
            if (i == 0) {
                DrawBird(birds[i], redBirdTexture);
            } else {
                DrawBird(birds[i], happyBirdTexture);
            }
        }

        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 140});
        DrawTextEx(customFont, "Game Over", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Game Over", 160, 0).x / 2, SCREEN_HEIGHT / 8}, 160, 0, WHITE);
        DrawTextEx(customFont, TextFormat("Score: %d", score), (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, TextFormat("Score: %d", score), 80, 0).x / 2, SCREEN_HEIGHT / 2}, 80, 0, WHITE);
        DrawTextEx(customFont, "Tap to restart", (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Tap to restart", 60, 0).x / 2, SCREEN_HEIGHT * 3 / 4}, 60, 0, WHITE);
    } else {
        DrawParallaxBackground();

        for (int i = 0; i < pipeCount; i++) {
            Rectangle topPipe = pipes[i].top;
            Rectangle bottomPipe = pipes[i].bottom;
            DrawRectangleRounded(topPipe, 0.1f, 16, (Color){40,255,40,255});
            DrawRectangleRounded(bottomPipe, 0.1f, 16, (Color){40,255,40,255});
        }

        for (int i = 0; i < 2; i++) {
            if (i == 0) {
                if(birds[i].isAlive) {
                    DrawBird(birds[i], redBirdTexture);
                }
            } else {
                if(birds[i].isAlive) {
                    DrawBird(birds[i], happyBirdTexture);
                }
            }

            if (!birds[i].isAlive) {
                double timeLeft = birds[i].respawnTime - gameTime;
                if (timeLeft > 0) {
                    DrawRectangleRounded((Rectangle){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Respawn in 0", 60, 0).x / 2 - 15, 20, MeasureTextEx(customFont, TextFormat("Respawn in %.0f", timeLeft), 60, 0).x+20, 62}, 0.1f, 16, (Color){0,0,0,90});
                    DrawTextEx(customFont, TextFormat("Respawn in %.0f", timeLeft + 0.25f), (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(customFont, "Respawn in 0", 60, 0).x / 2, 20}, 60, 0, WHITE);
                }

                float minTime = 2.0f;
                if (timeLeft <= minTime) {
                    float x = (minTime - timeLeft) / minTime;
                    Color ghostColor = i == 0 ? RED : YELLOW;
                    ghostColor.a = 255 * x * x;
                    DrawCircle(birds[i].position.x, SCREEN_HEIGHT / 2, 15 + 35 * (1 - x) * (1 - x), ghostColor);
                }
            }
        }

        DrawRectangleRounded((Rectangle){5, 10, MeasureTextEx(customFont, TextFormat("Score: %d", score), 60, 0).x + 15 ,60}, 0.1f, 16, (Color){0,0,0,90});
        DrawTextEx(customFont, TextFormat("Score: %d", score), (Vector2){10, 10}, 60, 0, WHITE);
    }

    EndDrawing();
}

int main(void) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Freaky Birds");
    InitAudioDevice();

    InitGame();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateGame(deltaTime);
        DrawGame();
    }

    CloseWindow();
    return 0;
}