#include "raylib.h"
#include "raymath.h"
#include <math.h>

#define NUM_KEYS 7
#define KEY_WIDTH 60
#define KEY_HEIGHT 200
#define MAX_BLOCKS 100
#define MAX_BALLS 40

typedef struct {
    Rectangle rect;
    bool pressed;
    int keyboardKey;
    Sound sound;
    Color color;
} PianoKey;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    bool active;
} Block;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    float radius;
    Color color;
    bool active;
} Ball;

const int screenWidth = 1280;
const int screenHeight = 720;


float sdBox(Vector2 p, Vector2 b) {
    Vector2 d = {fabsf(p.x) - b.x, fabsf(p.y) - b.y};
    return fminf(fmaxf(d.x, d.y), 0.0f) + sqrtf(fmaxf(d.x, 0.0f) * fmaxf(d.x, 0.0f) + fmaxf(d.y, 0.0f) * fmaxf(d.y, 0.0f));
}

float sdScene(Vector2 p, PianoKey* keys, Block* blocks) {
    float d = INFINITY;

    // Piano keys
    for (int i = 0; i < NUM_KEYS; i++) {
        Vector2 keyPos = {keys[i].rect.x + keys[i].rect.width / 2, keys[i].rect.y + keys[i].rect.height / 2};
        Vector2 keySize = {keys[i].rect.width / 2, keys[i].rect.height / 2};
        Vector2 relPos = {p.x - keyPos.x, p.y - keyPos.y};
        float keyDist = sdBox(relPos, keySize);
        d = fminf(d, keyDist);
    }

    // Blocks
    for (int i = 0; i < MAX_BLOCKS; i++) {
        if (blocks[i].active) {
            Vector2 blockPos = blocks[i].position;
            Vector2 blockSize = {blocks[i].size / 2, blocks[i].size / 2};
            Vector2 relPos = {p.x - blockPos.x, p.y - blockPos.y};
            float blockDist = sdBox(relPos, blockSize);
            d = fminf(d, blockDist);
        }
    }

    return d;
}

void HandleWallCollision(Ball* ball) {
    // Left and right walls
    if (ball->position.x - ball->radius < 0 || ball->position.x + ball->radius > screenWidth) {
        ball->velocity.x *= -1; // Reverse direction and reduce velocity
        ball->position.x = (ball->position.x - ball->radius < 0) ? ball->radius : screenWidth - ball->radius;
    }

    // Top wall
    if (ball->position.y - ball->radius < 0) {
        ball->velocity.y *= -1;
        ball->position.y = ball->radius;
    }

    if (ball->position.y > screenHeight - ball->radius) {
        ball->velocity.y *= -1;
        ball->position.y = screenHeight - ball->radius;
    }
}

void MoveBallsTowards(Ball* balls, Vector2 mousePosition) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            Vector2 direction = Vector2Subtract(mousePosition, balls[i].position);
            float len = Vector2Length(direction);
            direction = Vector2Normalize(direction);
            balls[i].velocity = Vector2Scale(direction, 600 + len);  // Adjust speed as needed
        }
    }
}

void HandleBallCollisions(Ball* balls) {
    for (int i = 0; i < MAX_BALLS; i++) {
        if (!balls[i].active) continue;

        for (int j = i + 1; j < MAX_BALLS; j++) {
            if (!balls[j].active) continue;

            Vector2 diff = Vector2Subtract(balls[j].position, balls[i].position);
            float dist = Vector2Length(diff);
            float minDist = balls[i].radius + balls[j].radius;

            if (dist < minDist) {
                Vector2 normal = Vector2Normalize(diff);
                float overlap = minDist - dist;

                // Adjust positions based on overlap
                balls[i].position = Vector2Subtract(balls[i].position, Vector2Scale(normal, overlap * 0.5f));
                balls[j].position = Vector2Add(balls[j].position, Vector2Scale(normal, overlap * 0.5f));

                // Reflect velocities
                Vector2 relativeVel = Vector2Subtract(balls[i].velocity, balls[j].velocity);
                float impulse = 2.0f * Vector2DotProduct(relativeVel, normal) / (balls[i].radius + balls[j].radius);

                balls[i].velocity = Vector2Subtract(balls[i].velocity, Vector2Scale(normal, impulse * balls[j].radius));
                balls[j].velocity = Vector2Add(balls[j].velocity, Vector2Scale(normal, impulse * balls[i].radius));
            }
        }
    }
}

int main(void) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Toplarr");
    InitAudioDevice();

    PianoKey keys[NUM_KEYS] = {
        {{400 + (KEY_WIDTH + 5) * 0, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_A, LoadSound("assets/c4.mp3"), MAROON},
        {{400 + (KEY_WIDTH + 5) * 1, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_S, LoadSound("assets/d4.mp3"), RED},
        {{400 + (KEY_WIDTH + 5) * 2, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_D, LoadSound("assets/e4.mp3"), ORANGE},
        {{400 + (KEY_WIDTH + 5) * 3, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_F, LoadSound("assets/f4.mp3"), YELLOW},
        {{400 + (KEY_WIDTH + 5) * 4, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_G, LoadSound("assets/g4.mp3"), GREEN},
        {{400 + (KEY_WIDTH + 5) * 5, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_H, LoadSound("assets/a4.mp3"), BLUE},
        {{400 + (KEY_WIDTH + 5) * 6, 720 - KEY_HEIGHT, KEY_WIDTH, KEY_HEIGHT}, false, KEY_J, LoadSound("assets/b4.mp3"), VIOLET}
    };

    Block blocks[MAX_BLOCKS] = {0};
    Ball balls[MAX_BALLS] = {0};

    for (int j = 0; j < MAX_BALLS; j++) {
        if (!balls[j].active) {
            balls[j].position = (Vector2){GetRandomValue(0, 1280), GetRandomValue(0, 400)};
            balls[j].velocity = Vector2Scale(Vector2Normalize((Vector2){GetRandomValue(-100, 100), GetRandomValue(-100, 100)}), 500);
            balls[j].radius = 10;
            balls[j].color = BLACK;
            balls[j].active = true;
        }
    }

    while (!WindowShouldClose()) {

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePosition = GetMousePosition();
            MoveBallsTowards(balls, mousePosition);
        }

        for (int i = 0; i < NUM_KEYS; i++) {
            if (IsKeyPressed(keys[i].keyboardKey)) {
                keys[i].pressed = true;
                PlaySound(keys[i].sound);
                for (int j = 0; j < MAX_BLOCKS; j++) {
                    if (!blocks[j].active) {
                        blocks[j].position = (Vector2){keys[i].rect.x + KEY_WIDTH/2, keys[i].rect.y};
                        blocks[j].velocity = (Vector2){0, -2};
                        blocks[j].color = keys[i].color;
                        blocks[j].size = KEY_WIDTH;
                        blocks[j].active = true;
                        MoveBallsTowards(balls, blocks[j].position);
                        break;
                    }
                }
            } else if (IsKeyReleased(keys[i].keyboardKey)) {
                keys[i].pressed = false;
            }
        }


        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (blocks[i].active) {
                blocks[i].position.y += blocks[i].velocity.y;
                if (blocks[i].size < 1 || blocks[i].position.y + KEY_WIDTH < 0) {
                    blocks[i].active = false;
                }
            }
        }


        float dt = GetFrameTime();
        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active) {

                balls[i].velocity =  Vector2Scale(Vector2Normalize(balls[i].velocity), 600);
                balls[i].position.x += balls[i].velocity.x * dt;
                balls[i].position.y += balls[i].velocity.y * dt;

                HandleWallCollision(&balls[i]);

                // Piano key collision (using existing SDF method)
                float d = sdScene(balls[i].position, keys, blocks);
                if (d < balls[i].radius) {
                    float eps = 0.01f;
                    float dx = sdScene((Vector2){balls[i].position.x + eps, balls[i].position.y}, keys, blocks) - d;
                    float dy = sdScene((Vector2){balls[i].position.x, balls[i].position.y + eps}, keys, blocks) - d;
                    Vector2 normal = Vector2Normalize((Vector2){dx, dy});

                    Vector2 reflected = Vector2Subtract(balls[i].velocity,
                        Vector2Scale(normal, 2 * Vector2DotProduct(balls[i].velocity, normal)));

                    balls[i].velocity = Vector2Scale(reflected, 1);
                    balls[i].position = Vector2Add(balls[i].position, Vector2Scale(normal, balls[i].radius - d));
                }
            }
        }

        HandleBallCollisions(balls);

        BeginDrawing();
        ClearBackground(RAYWHITE);


        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (blocks[i].active) {
                DrawRectangle(blocks[i].position.x - blocks[i].size/2, blocks[i].position.y - blocks[i].size/2,
                              blocks[i].size, blocks[i].size, blocks[i].color);
            }
        }

        for (int i = 0; i < MAX_BALLS; i++) {
            if (balls[i].active) {
                DrawCircleV(balls[i].position, balls[i].radius, balls[i].color);
            }
        }


        for (int i = 0; i < NUM_KEYS; i++) {
            DrawRectangleRec(keys[i].rect, keys[i].pressed ? keys[i].color : LIGHTGRAY);
            DrawRectangleLinesEx(keys[i].rect, 2, BLACK);
        }

        EndDrawing();
    }


    for (int i = 0; i < NUM_KEYS; i++) {
        UnloadSound(keys[i].sound);
    }

    CloseAudioDevice();
    CloseWindow();

    return 0;
}