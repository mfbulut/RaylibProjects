#include "raylib.h"
#include "raymath.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
#define PLAYER_RADIUS 25
#define BALL_RADIUS 10
#define GOAL_WIDTH 200

typedef struct Player {
    Vector2 position;
    Vector2 speed;
    Vector2 mov;
    int score;
    bool canDash;
    float dashCooldown;
} Player;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
} Ball;

void DrawFootballField() {
    ClearBackground((Color){34, 139, 34, 255});

    DrawLineEx((Vector2){SCREEN_WIDTH/2, 0}, (Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT}, 4, WHITE);

    DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 80, WHITE);
    DrawCircleLines(SCREEN_WIDTH/2, SCREEN_HEIGHT/2, 81, WHITE);
    DrawCircleV((Vector2){SCREEN_WIDTH/2, SCREEN_HEIGHT/2}, 3, WHITE);

    int goalAreaWidth = 180;
    int goalAreaHeight = 280;

    DrawRectangleLinesEx((Rectangle){0, (SCREEN_HEIGHT - goalAreaHeight)/2, goalAreaWidth, goalAreaHeight}, 2, WHITE);
    DrawRectangleLinesEx((Rectangle){SCREEN_WIDTH - goalAreaWidth, (SCREEN_HEIGHT - goalAreaHeight)/2, goalAreaWidth, goalAreaHeight}, 2, WHITE);
    DrawRectangleLinesEx((Rectangle){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT}, 2, WHITE);
}

void UpdatePlayer(Player *player, int upKey, int downKey, int leftKey, int rightKey, int dashKey) {
    Vector2 mov = {0};
    if (IsKeyDown(upKey)) mov.y -= 1.0f;
    if (IsKeyDown(downKey)) mov.y += 1.0f;
    if (IsKeyDown(leftKey)) mov.x -= 1.0f;
    if (IsKeyDown(rightKey)) mov.x += 1.0f;

    player->position = Vector2Add(player->position, Vector2Scale(mov, 3));
    player->mov = mov;

    if (IsKeyPressed(dashKey) && player->canDash && (mov.x != 0 || mov.y != 0)) {
        player->speed.x = (IsKeyDown(rightKey) - IsKeyDown(leftKey)) * 5.0f;
        player->speed.y = (IsKeyDown(downKey) - IsKeyDown(upKey)) * 5.0f;
        player->canDash = false;
        player->dashCooldown = 1.0f;
    }

    player->position.x += player->speed.x;
    player->position.y += player->speed.y;

    player->speed.x *= 0.9;
    player->speed.y *= 0.9;

    if (!player->canDash) {
        player->dashCooldown -= GetFrameTime();
        if (player->dashCooldown <= 0) {
            player->canDash = true;
            player->speed = (Vector2){0, 0};
        }
    }

    if (player->position.x - PLAYER_RADIUS < 0) player->position.x = PLAYER_RADIUS;
    if (player->position.x + PLAYER_RADIUS > SCREEN_WIDTH) player->position.x = SCREEN_WIDTH - PLAYER_RADIUS;
    if (player->position.y - PLAYER_RADIUS < 0) player->position.y = PLAYER_RADIUS;
    if (player->position.y + PLAYER_RADIUS > SCREEN_HEIGHT) player->position.y = SCREEN_HEIGHT - PLAYER_RADIUS;
}

void UpdateBall(Ball *ball, Player *player1, Player *player2) {
    ball->position.x += ball->speed.x;
    ball->position.y += ball->speed.y;

    if(Vector2Length(player1->speed) > 0.0f)
        ball->speed = Vector2Scale(ball->speed, 0.97f);

    if (ball->position.x - BALL_RADIUS < 0) {
        if (ball->position.y > (SCREEN_HEIGHT - GOAL_WIDTH) / 2 && ball->position.y < (SCREEN_HEIGHT + GOAL_WIDTH) / 2) {
            player2->score++;
            ball->position = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            ball->speed = (Vector2){ 0, 0 };
        } else {
            ball->position.x = BALL_RADIUS;
            ball->speed.x = -ball->speed.x;
        }
    }

    if (ball->position.x + BALL_RADIUS > SCREEN_WIDTH) {
        if (ball->position.y > (SCREEN_HEIGHT - GOAL_WIDTH) / 2 && ball->position.y < (SCREEN_HEIGHT + GOAL_WIDTH) / 2) {
            player1->score++;
            ball->position = (Vector2){ SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            ball->speed = (Vector2){ 0, 0 };
        } else {
            ball->position.x = SCREEN_WIDTH - BALL_RADIUS;
            ball->speed.x = -ball->speed.x;
        }
    }

    if (ball->position.y - BALL_RADIUS < 0 || ball->position.y + BALL_RADIUS > SCREEN_HEIGHT) {
        ball->speed.y = -ball->speed.y;
        ball->position.y = Clamp(ball->position.y, BALL_RADIUS, SCREEN_HEIGHT - BALL_RADIUS);
    }

    if (CheckCollisionCircles(ball->position, BALL_RADIUS, player1->position, PLAYER_RADIUS)) {
        Vector2 dir = Vector2Subtract(ball->position, player1->position);
        float len = Vector2Length(dir);
        dir = Vector2Scale(dir, 1 / len);
        ball->position = Vector2Add(ball->position, Vector2Scale(dir, BALL_RADIUS + PLAYER_RADIUS - len));

        if(!player1->canDash || Vector2Length(player1->speed) > 0) {
            ball->speed = Vector2Scale(Vector2Normalize(player1->speed), 10.0f);
        }
        else {
            ball->speed = Vector2Scale(player1->mov, 2.0f);
        }
    }
    if (CheckCollisionCircles(ball->position, BALL_RADIUS, player2->position, PLAYER_RADIUS)) {
        Vector2 dir = Vector2Subtract(ball->position, player2->position);
        float len = Vector2Length(dir);
        dir = Vector2Scale(dir, 1 / len);
        ball->position = Vector2Add(ball->position, Vector2Scale(dir, BALL_RADIUS + PLAYER_RADIUS - len));

        if(!player2->canDash || Vector2Length(player2->speed) > 0) {
            ball->speed = Vector2Scale(Vector2Normalize(player2->speed), 10.0f);
        }
        else {
            ball->speed = Vector2Scale(player2->mov, 2.0f);
        }
    }
}

void HandlePlayerCollisions(Player *player1, Player *player2) {
    if (CheckCollisionCircles(player1->position, PLAYER_RADIUS, player2->position, PLAYER_RADIUS)) {
        Vector2 dir = Vector2Subtract(player2->position, player1->position);
        float len = Vector2Length(dir);
        dir = Vector2Normalize(dir);
        player1->position = Vector2Subtract(player1->position, Vector2Scale(dir, (PLAYER_RADIUS * 2 - len) / 2));
        player2->position = Vector2Add(player2->position, Vector2Scale(dir, (PLAYER_RADIUS * 2 - len) / 2));
    }
}

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Local Multiplayer Football Game");
    SetTargetFPS(60);

    Player player1 = { .position = { 100, SCREEN_HEIGHT / 2 }, .speed = { 0, 0 }, .score = 0, .canDash = true, .dashCooldown = 0 };
    Player player2 = { .position = { SCREEN_WIDTH - 100, SCREEN_HEIGHT / 2 }, .speed = { 0, 0 }, .score = 0, .canDash = true, .dashCooldown = 0 };
    Ball ball = { .position = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }, .speed = { 0, 0 } };

    while (!WindowShouldClose()) {

        UpdatePlayer(&player1, KEY_W, KEY_S, KEY_A, KEY_D, KEY_SPACE);
        UpdatePlayer(&player2, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_RIGHT_CONTROL);

        HandlePlayerCollisions(&player1, &player2);

        UpdateBall(&ball, &player1, &player2);

        BeginDrawing();

        DrawFootballField();

        DrawCircleV(player1.position, PLAYER_RADIUS, BLUE);
        DrawRing(player1.position, PLAYER_RADIUS - 3, PLAYER_RADIUS, 0, 360.0f * (1.0f - player1.dashCooldown) , 32, WHITE);
        DrawCircleV(player2.position, PLAYER_RADIUS, RED);
        DrawRing(player2.position, PLAYER_RADIUS - 3, PLAYER_RADIUS, 0, 360.0f * (1.0f - player2.dashCooldown) , 32, WHITE);
        DrawCircleV(ball.position, BALL_RADIUS, WHITE);

        DrawLineEx((Vector2){0, (SCREEN_HEIGHT - GOAL_WIDTH) / 2}, (Vector2){0, (SCREEN_HEIGHT + GOAL_WIDTH) / 2}, 8, RED);
        DrawLineEx((Vector2){SCREEN_WIDTH - 1, (SCREEN_HEIGHT - GOAL_WIDTH) / 2}, (Vector2){SCREEN_WIDTH - 1, (SCREEN_HEIGHT + GOAL_WIDTH) / 2}, 8, RED);

        DrawText(TextFormat("Player 1: %d", player1.score), 10, 10, 30, WHITE);
        DrawText(TextFormat("Player 2: %d", player2.score), SCREEN_WIDTH - 200, 10, 30, WHITE);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}