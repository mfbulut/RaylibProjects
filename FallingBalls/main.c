#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define BALL_RADIUS 10
#define GRID_ROWS 50
#define GRID_COLS 12
#define GRID_BLOCK_WIDTH 60
#define GRID_BLOCK_HEIGHT 30
#define GRID_SPACING 5
#define GRAVITY 150.0f
#define BALL_COUNT 10
#define CAMERA_SCROLL_SPEED 40.0f
#define BALL_SPAWN_DELAY 1.0f
#define VELOCITY_DAMPING 0.8f

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Vector2 prevPosition;
    float radius;
    bool active;
    Color color;
} Ball;

typedef struct {
    Rectangle rect;
    Color color;
    bool active;
    int health;
} GridBlock;

typedef struct {
    bool collision;
    Vector2 normal;
    float depth;
} CollisionInfo;

Ball balls[BALL_COUNT];
GridBlock blocks[GRID_ROWS][GRID_COLS];
Camera2D camera = { 0 };
float spawnTimer = 0.0f;

void InitGame();
void UpdateGame();
void DrawGame();
void SpawnBall();
void UpdateBall(Ball *ball, float deltaTime);
void DrawBlock(GridBlock block);
void HandleBallBallCollisions();

CollisionInfo SDFCollisionBallGrid(Ball ball, GridBlock block);

float CalculateAverageBallPosition() {
    float avgPos = 0;
    int activeBalls = 0;

    for (int i = 0; i < BALL_COUNT; i++) {
        if (balls[i].active) {
            avgPos += balls[i].position.y;
            activeBalls++;
        }
    }

    if (activeBalls > 0) {
        avgPos /= (activeBalls);
    }

    return avgPos;
}

int main(void) {
    SetConfigFlags(FLAG_VSYNC_HINT);
    SetConfigFlags(FLAG_WINDOW_ALWAYS_RUN);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Falling Balls");

    InitGame();

    while (!WindowShouldClose()) {
        UpdateGame();

        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode2D(camera);
        DrawGame();
        EndMode2D();

        EndDrawing();
    }

    CloseWindow();
}

void InitGame() {
    for (int i = 0; i < BALL_COUNT; i++) {
        balls[i].active = false;
        balls[i].radius = BALL_RADIUS;
    }

    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {

            blocks[i][j].rect = (Rectangle){
                j * (GRID_BLOCK_WIDTH + GRID_SPACING) + GRID_SPACING,
                SCREEN_HEIGHT + i * (GRID_BLOCK_HEIGHT + GRID_SPACING),
                GRID_BLOCK_WIDTH,
                GRID_BLOCK_HEIGHT
            };

            int pattern = (i + j) % 5;
            Color colors[5] = { BLUE, GREEN, GOLD, PURPLE, SKYBLUE };
            blocks[i][j].color = colors[pattern];

            int chance = GetRandomValue(0, 100);
            if (chance < 20) {
                blocks[i][j].active = false;
            } else {
                blocks[i][j].active = true;
                blocks[i][j].health = (chance > 90) ? 2 : 1;
            }
        }
    }

    camera.target = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    camera.offset = (Vector2){ SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
}

void UpdateGame() {
    float deltaTime = GetFrameTime();

    spawnTimer += deltaTime;
    if (spawnTimer >= BALL_SPAWN_DELAY) {
        spawnTimer = 0.0f;
        SpawnBall();
    }

    for (int i = 0; i < BALL_COUNT; i++) {
        if (balls[i].active) {
            UpdateBall(&balls[i], deltaTime);

            // Check if the ball is out of view (top, bottom, or sides)
            if (balls[i].position.y > camera.target.y + SCREEN_HEIGHT * 1.5f ||
                balls[i].position.y < camera.target.y - SCREEN_HEIGHT * 1.5f ||
                balls[i].position.x < -SCREEN_WIDTH ||
                balls[i].position.x > SCREEN_WIDTH * 2) {

                balls[i].active = false;
            }
        }
    }

    HandleBallBallCollisions();

    float targetPositionY = CalculateAverageBallPosition() + 200;

    float smoothFactor = 2.0f * deltaTime;
    camera.target.y = Lerp(camera.target.y, targetPositionY, smoothFactor);

    camera.target.y += CAMERA_SCROLL_SPEED * 0.2f * deltaTime;
}

void UpdateBall(Ball *ball, float deltaTime) {
    ball->prevPosition = ball->position;
    ball->velocity.y += GRAVITY * deltaTime;
    ball->position.x += ball->velocity.x * deltaTime;
    ball->position.y += ball->velocity.y * deltaTime;

    if (ball->position.x - ball->radius < 0) {
        ball->position.x = ball->radius;
        ball->velocity.x = -ball->velocity.x * VELOCITY_DAMPING;
    }
    if (ball->position.x + ball->radius > SCREEN_WIDTH) {
        ball->position.x = SCREEN_WIDTH - ball->radius;
        ball->velocity.x = -ball->velocity.x * VELOCITY_DAMPING;
    }

    for (int i = 0; i < GRID_ROWS; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            CollisionInfo collision = SDFCollisionBallGrid(*ball, blocks[i][j]);
            if (collision.collision) {
                ball->position = Vector2Add(ball->position, Vector2Scale(collision.normal, collision.depth));
                ball->velocity = Vector2Subtract(ball->velocity, Vector2Scale(collision.normal, 1.9f * Vector2DotProduct(ball->velocity, collision.normal)));

                blocks[i][j].health--;
                if (blocks[i][j].health <= 0) {
                    blocks[i][j].active = false;
                } else {
                    blocks[i][j].color = ColorBrightness(blocks[i][j].color , 0.7);
                }
            }
        }
    }
}

void DrawGame() {
    int startRow = fmax(0, (camera.target.y - SCREEN_HEIGHT) / (GRID_BLOCK_HEIGHT + GRID_SPACING) - 10);
    int endRow = fmin(GRID_ROWS - 1, (camera.target.y + SCREEN_HEIGHT) / (GRID_BLOCK_HEIGHT + GRID_SPACING) + 5);

    for (int i = startRow; i <= endRow; i++) {
        for (int j = 0; j < GRID_COLS; j++) {
            if (blocks[i][j].active) {
                DrawBlock(blocks[i][j]);
            }
        }
    }

    for (int i = 0; i < BALL_COUNT; i++) {
        if (balls[i].active) {
            DrawCircleV((Vector2){balls[i].position.x + 3, balls[i].position.y + 3},
                       balls[i].radius, ColorAlpha(BLACK, 0.2f));

            DrawCircleV(balls[i].position, balls[i].radius, RED);

            DrawCircleV((Vector2){balls[i].position.x - balls[i].radius*0.3f,
                                balls[i].position.y - balls[i].radius*0.3f},
                       balls[i].radius*0.3f, ColorAlpha(WHITE, 0.7f));

        }
    }

    for (int y = ((int)camera.target.y - SCREEN_HEIGHT/2) / 100 * 100;
         y < ((int)camera.target.y + SCREEN_HEIGHT/2) / 100 * 100 + SCREEN_HEIGHT;
         y += 100) {
        DrawLine(0, y, SCREEN_WIDTH, y, ColorAlpha(GRAY, 0.2f));
        DrawText(TextFormat("%d", y), 5, y, 10, ColorAlpha(GRAY, 0.5f));
    }
}

void DrawBlock(GridBlock block) {
    DrawRectangle(block.rect.x + 3, block.rect.y + 3, block.rect.width, block.rect.height, ColorAlpha(BLACK, 0.2f));
    DrawRectangleRec(block.rect, block.color);
    DrawRectangleLinesEx(block.rect, 1, DARKGRAY);

    if (block.health > 1) {
        DrawRectangle(block.rect.x + block.rect.width/2 - 5, block.rect.y + block.rect.height/2 - 5, 10, 10, WHITE);
    }
}

void SpawnBall() {
    for (int i = 0; i < BALL_COUNT; i++) {
        if (!balls[i].active) {

            float cameraTopY = camera.target.y - SCREEN_HEIGHT/3;

            balls[i].position = (Vector2){
                GetRandomValue(balls[i].radius, SCREEN_WIDTH - balls[i].radius),
                cameraTopY - balls[i].radius * 2
            };

            balls[i].prevPosition = balls[i].position;

            balls[i].velocity = (Vector2){
                GetRandomValue(-100, 100),
                GetRandomValue(0, 150)
            };

            balls[i].radius = BALL_RADIUS + GetRandomValue(-2, 2);

            int colorIndex = GetRandomValue(0, 3);
            Color ballColors[4] = { RED, ORANGE, MAROON, PINK };
            balls[i].color = ballColors[colorIndex];

            balls[i].active = true;
            break;
        }
    }
}

void HandleBallBallCollisions() {
    for (int i = 0; i < BALL_COUNT; i++) {
        if (!balls[i].active) continue;
        for (int j = i + 1; j < BALL_COUNT; j++) {
            if (!balls[j].active) continue;

            Vector2 diff = Vector2Subtract(balls[j].position, balls[i].position);
            float dist = Vector2Length(diff);
            float minDist = balls[i].radius + balls[j].radius;

            if (dist < minDist && dist > 0.0f) {
                Vector2 normal = Vector2Scale(diff, 1.0f / dist);
                float penetration = minDist - dist;

                balls[i].position = Vector2Subtract(balls[i].position, Vector2Scale(normal, penetration * 0.5f));
                balls[j].position = Vector2Add(balls[j].position, Vector2Scale(normal, penetration * 0.5f));

                Vector2 relativeVelocity = Vector2Subtract(balls[j].velocity, balls[i].velocity);
                float velocityAlongNormal = Vector2DotProduct(relativeVelocity, normal);
                if (velocityAlongNormal > 0) continue;

                float e = 0.8f;
                float j2 = -(1 + e) * velocityAlongNormal / 2;

                Vector2 impulse = Vector2Scale(normal, j2);
                balls[i].velocity = Vector2Subtract(balls[i].velocity, impulse);
                balls[j].velocity = Vector2Add(balls[j].velocity, impulse);
            }
        }
    }
}

CollisionInfo SDFCollisionBallGrid(Ball ball, GridBlock block) {
    CollisionInfo collision = {false, {0, 0}, 0};

    if (!block.active) return collision;

    Vector2 closestPoint;
    closestPoint.x = fmax(block.rect.x, fmin(ball.position.x, block.rect.x + block.rect.width));
    closestPoint.y = fmax(block.rect.y, fmin(ball.position.y, block.rect.y + block.rect.height));

    Vector2 diff = Vector2Subtract(ball.position, closestPoint);
    float dist = Vector2Length(diff);

    if (dist < ball.radius) {
        collision.collision = true;
        collision.normal = Vector2Normalize(diff);
        collision.depth = ball.radius - dist;
    }
    return collision;
}