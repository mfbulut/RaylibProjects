#include "raylib.h"
#include "raymath.h"
#include "sdf.h"

const int screenWidth = 1280;
const int screenHeight = 720;

#define MAX_SHAPES 13
Shape shapes[MAX_SHAPES] = {
    (Shape){ SHAPE_BOX, {-1.0f,  0.55f}, {0.05f, 0.4f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, { 1.0f,  0.35f}, {0.05f, 0.25f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, { 0.0f,  1.0f}, {1.05f, 0.05f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, { 0.0f, 0.1f}, {1.05f, 0.05f}, 0, (Color) { 150, 75, 0, 255 } },

    (Shape){ SHAPE_ORIENTED_BOX, {2.32f, 1.745f}, {0.05f, 1.5f}, -(PI / 3), (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_ORIENTED_BOX, {2.32f, 1.30f}, {0.05f, 1.5f}, -(PI / 3), (Color) { 150, 75, 0, 255 } },

    (Shape){ SHAPE_BOX, {  1.0f + 4.6f,  1.5f + 0.35f}, {0.05f, 0.25f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, { -1.0f + 4.6f,  1.5f + 0.35f}, {0.05f, 0.25f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, {  0.0f + 4.6f,  1.5f + 1.0f}, {1.05f, 0.05f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, {  0.0f + 4.6f,  1.5f + 0.1f}, {1.05f, 0.05f}, 0, (Color) { 150, 75, 0, 255 } },


    (Shape){ SHAPE_BOX, {  0.0f + 8.0f, 2.5f}, {3.0f, 0.05f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, {  0.0f + 7.5f, 2.05f}, {0.2f, 0.4f}, 0, (Color) { 150, 75, 0, 255 } },
    (Shape){ SHAPE_BOX, {  0.0f + 9.5f, 2.25f}, {0.15f, 0.2f}, 0, (Color) { 150, 75, 0, 255 } },
};

typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 prevPos;
    float radius;
} Player;

typedef enum {
    BULLET,
    COIN,
    EXPLOSIVE
} ProjectileType;

typedef struct {
    ProjectileType type;
    Vector2 pos;
    Vector2 vel;
    bool active;
} Projectile;

typedef struct {
    Shape shape;
    Vector2 vel;
    bool active;
    float health;
} Enemy;

#define MAX_PROJECTILES 256
#define MAX_ENEMIES 10

int projectileCount = 0;
Projectile projectiles[MAX_PROJECTILES];

Enemy enemies[MAX_ENEMIES];
int enemyCount = 0;

Camera2D camera;

void InitProjectile(ProjectileType type, Vector2 pos, Vector2 vel) {
    projectiles[projectileCount].type = type;
    projectiles[projectileCount].pos = pos;
    projectiles[projectileCount].vel = vel;
    projectiles[projectileCount].active = true;
    projectileCount = (projectileCount+1) % MAX_PROJECTILES;
}

void UpdateProjectile(int index, float deltaTime) {
    if (!projectiles[index].active) return;

    switch (projectiles[index].type) {
        case BULLET:
            projectiles[index].pos = Vector2Add(projectiles[index].pos, Vector2Scale(projectiles[index].vel, deltaTime));
            break;
        case COIN:
            projectiles[index].vel = Vector2Add(projectiles[index].vel, (Vector2){ 0, 6.0f * deltaTime });
            projectiles[index].pos = Vector2Add(projectiles[index].pos, Vector2Scale(projectiles[index].vel, deltaTime));
            break;
        case EXPLOSIVE:
            // Explosive behavior TODO
            break;
    }

    // Collision detection with shapes
    for (int i = 0; i < MAX_SHAPES; i++) {
        float dist = SDF(shapes[i], projectiles[index].pos);
        if (dist < 0.02f) {
            projectiles[index].active = false;
            break;
        }
    }

    // Collision detection between projectiles
    for (int i = 0; i < projectileCount; i++) {
        if (i != index && projectiles[i].active) {
            float distance = Vector2Distance(projectiles[index].pos, projectiles[i].pos);
            if (distance < 0.06f) {
                if (projectiles[index].type == BULLET && projectiles[i].type == COIN) {
                    // Redirect bullet to the nearest enemy
                    float minDist = 1000.0f;
                    Vector2 targetDir = { 0 };
                    for (int j = 0; j < enemyCount; j++) {
                        if (enemies[j].active) {
                            float enemyDist = Vector2Distance(projectiles[index].pos, enemies[j].shape.pos);
                            if (enemyDist < minDist) {
                                minDist = enemyDist;
                                targetDir = Vector2Normalize(Vector2Subtract(enemies[j].shape.pos, projectiles[index].pos));
                            }
                        }
                    }
                    if (minDist < 1000.0f) {
                        projectiles[index].vel = Vector2Scale(targetDir, 8.0f); // Adjust bullet speed as needed
                        projectiles[i].active = false; // Deactivate the coin
                    }
                }
                break;
            }
        }
    }
}

void DrawProjectile(int index) {
    if (!projectiles[index].active) return;

    switch (projectiles[index].type) {
        case BULLET:
            DrawCircleV(projectiles[index].pos, 0.01f, WHITE);
            break;
        case COIN:
            DrawCircleV(projectiles[index].pos, 0.02f, GOLD);
            break;
        case EXPLOSIVE:
            DrawCircleV(projectiles[index].pos, 0.02f, ORANGE);
            break;
    }
}

void InitEnemy(Shape shape, Vector2 vel, float health) {
    enemies[enemyCount].shape = shape;
    enemies[enemyCount].vel = vel;
    enemies[enemyCount].active = true;
    enemies[enemyCount].health = health;
    enemyCount++;
}

void UpdateEnemy(int index, float deltaTime) {
    if (!enemies[index].active) return;

    // Update enemy position
    enemies[index].shape.pos = Vector2Add(enemies[index].shape.pos, Vector2Scale(enemies[index].vel, deltaTime));

    for (int i = 0; i < MAX_SHAPES; i++) {
        float dist = SDF(shapes[i], enemies[index].shape.pos);
        if (dist < enemies[index].shape.size.x) {
            enemies[index].vel = Vector2Scale(enemies[index].vel, -1);  // Reverse direction on collision
            break;
        }
    }

    // Collision detection with projectiles
    for (int i = 0; i < projectileCount; i++) {
        if (projectiles[i].active && projectiles[i].type == BULLET) {
            float distance = SDF(enemies[index].shape, projectiles[i].pos);
            if (distance < 0.03f) {
                projectiles[i].active = false;
                enemies[index].health -= 10;  // Decrease health on hit
                if (enemies[index].health <= 0) {
                    enemies[index].active = false;
                }
                break;
            }
        }
    }
}

void DrawEnemy(int index) {
    if (!enemies[index].active) return;

    // Draw the enemy shape
    DrawShape(enemies[index].shape);

    // Draw the health bar above the enemy
    float healthBarWidth = 0.12f; // Width of the health bar
    float healthBarHeight = 0.015f; // Height of the health bar
    float healthPercentage = enemies[index].health / 100.0f; // Assuming max health is 100
    Vector2 healthBarPos = Vector2Add(enemies[index].shape.pos, (Vector2){ -healthBarWidth / 2, -enemies[index].shape.size.y / 2 - healthBarHeight + -0.075 });

    // Draw the background of the health bar (gray)
    DrawRectangleV(healthBarPos, (Vector2){ healthBarWidth, healthBarHeight }, DARKGRAY);

    // Draw the foreground of the health bar (red)
    DrawRectangleV(healthBarPos, (Vector2){ healthBarWidth * healthPercentage, healthBarHeight }, RED);
}


void UpdateCameraPlayerBoundsPush(Camera2D *camera, Player *player, int width, int height)
{
    static Vector2 bbox = { 0.2f, 0.1f };
    float yOffset = 0.3f;

    Vector2 bboxWorldMin = GetScreenToWorld2D((Vector2){ (1 - bbox.x)*0.5f*width, (1 - bbox.y)*0.5f*height }, *camera);
    Vector2 bboxWorldMax = GetScreenToWorld2D((Vector2){ (1 + bbox.x)*0.5f*width, (1 + bbox.y)*0.5f*height }, *camera);
    camera->offset = (Vector2){ (1 - bbox.x)*0.5f * width, (1 - bbox.y)*0.5f*height };

    if (player->pos.x < bboxWorldMin.x) camera->target.x = player->pos.x;
    if (player->pos.y - yOffset < bboxWorldMin.y) camera->target.y = player->pos.y - yOffset;
    if (player->pos.x > bboxWorldMax.x) camera->target.x = bboxWorldMin.x + (player->pos.x - bboxWorldMax.x);
    if (player->pos.y - yOffset > bboxWorldMax.y) camera->target.y = bboxWorldMin.y + (player->pos.y - yOffset - bboxWorldMax.y);
}

void UpdatePlayer(Player *player, float deltaTime) {
    // Player movement and physics update code
    player->vel = Vector2Add(player->vel, Vector2Scale((Vector2){ 0.0f, 2.0f }, deltaTime));
    player->prevPos = player->pos;
    player->pos = Vector2Add(player->pos, Vector2Scale(player->vel, deltaTime));

    // Player-ground collision and control logic (existing code)
    static float isGrounded = 0.0f;
    int isWall = false;
    for (int j = 0; j < MAX_SHAPES; j++) {
        float newDist = SDF(shapes[j], player->pos);
        for (int i = 0; i < 32; i++) {
            newDist = fmin(newDist, SDF(shapes[j], Vector2Add(player->pos, (Vector2){ 0, -i / 320.0f })));
        }
        if (newDist < player->radius) {
            Vector2 normal = computeNormal(shapes[j], player->pos, 0.01f);
            player->pos = Vector2Add(player->pos, Vector2Scale(normal, player->radius - newDist));
            isGrounded = 0.1f;
            if (fabs(normal.y) < 0.1f) {
                if (normal.x > 0)
                    isWall = 1;
                else
                    isWall = -1;
            }
        }
    }

    // Player velocity update (existing code)
    if (deltaTime != 0.0f)
        player->vel = Vector2Scale(Vector2Subtract(player->pos, player->prevPos), 1 / deltaTime);

    player->vel.x *= 0.95f;
    if (IsKeyDown(KEY_A))
        player->vel.x = fmax(player->vel.x - (10.0f * deltaTime), -1.0f);
    if (IsKeyDown(KEY_D))
        player->vel.x = fmin(player->vel.x + (10.0f * deltaTime), 1.0f);

    // Player jumping logic (existing code)
    static float jumpBuffer = 0.0f;
    if (IsKeyPressed(KEY_SPACE))
        jumpBuffer = 0.1f;

    if (isGrounded > 0.0f && jumpBuffer > 0.0f) {
        if (isWall < 0) {
            player->vel.x = -2.4f;
            player->vel.y = -1.6f;
        }
        else if (isWall > 0) {
            player->vel.x = 2.4f;
            player->vel.y = -1.6f;
        }
        else {
            player->vel.y = -1.2f;
        }
    }

    if (IsKeyUp(KEY_SPACE))
        player->vel.y += 1.0f * deltaTime;

    jumpBuffer -= deltaTime;
    isGrounded -= deltaTime;

    static float cooldown = 0.0f;
    if (IsKeyDown(KEY_LEFT_CONTROL) && cooldown <= 0.0f) {
        if (isGrounded > 0.0f)
            player->vel.y = 1.0f;
        else
            player->vel.y = 5.0f;
    }
    cooldown -= deltaTime;

    static float dash = 0.0f;
    static float dashDir = 0.0f;

    if (IsKeyDown(KEY_A) && IsKeyPressed(KEY_LEFT_SHIFT)) {
        dash = 0.1f;
        dashDir = -4.0f;
    }
    if (IsKeyDown(KEY_D) && IsKeyPressed(KEY_LEFT_SHIFT)) {
        dash = 0.1f;
        dashDir = 4.0f;
    }

    if (dash > 0.0f) {
        player->vel.x = dashDir;
        dash -= deltaTime;
    }

    // Collision detection with enemies
    for (int i = 0; i < enemyCount; i++) {
        if (enemies[i].active) {
            float distance = SDF(enemies[i].shape, player->pos);
            if (distance < player->radius) {
                if(player->pos.y > enemies[i].shape.pos.y ) {
                    continue;
                }
                // Apply bounce effect
                int mult = 1;
                if(IsKeyDown(KEY_LEFT_CONTROL)) {
                    mult = 2;
                    cooldown = 0.4f;
                }

                player->pos = Vector2Add(player->pos, (Vector2){0.0f, distance - player->radius});
                player->vel = (Vector2){0.0f, -1.0f * mult};

                // Damage the enemy
                enemies[i].health -= 10 * mult;
                if (enemies[i].health <= 0) {
                    enemies[i].active = false;
                }
                break;
            }
        }
    }

    Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);
    Vector2 direction = Vector2Normalize(Vector2Subtract(mousePos, Vector2Add(player->pos, (Vector2){ 0, -0.05f })));

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (projectileCount < MAX_PROJECTILES) {
            InitProjectile(BULLET, Vector2Add(player->pos, (Vector2){ direction.x / 20, direction.y / 20 - 0.05f }), Vector2Scale(direction, 6.0f));
        }
    }
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        if (projectileCount < MAX_PROJECTILES) {
            Vector2 bvel = Vector2Scale(direction, 1.0f);
            bvel.y = -2.0f;
            InitProjectile(COIN, Vector2Add(player->pos, (Vector2){ direction.x / 20, direction.y / 20 - 0.05f }), bvel);
        }
    }
}

int main() {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "Platformer - Shooter");

    Player player = { (Vector2){ 0, 0.8f }, (Vector2){ 0, 0 }, (Vector2){ 0, 0 }, 0.05f };
    camera.offset = (Vector2){ screenWidth / 2, screenHeight / 2 };
    camera.target = (Vector2){ 0, 0 };
    camera.rotation = 0.0f;
    camera.zoom = screenHeight / 2.0f;

    // Initialize enemies
    InitEnemy((Shape){ SHAPE_BOX, {8.5f, 2.35f}, {0.05f, 0.1f}, 0, (Color) { 160, 60, 160, 255 } }, (Vector2){ -0.5f, 0 }, 100);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        UpdatePlayer(&player, deltaTime);
        UpdateCameraPlayerBoundsPush(&camera, &player, screenWidth, screenHeight);

        for (int i = 0; i < projectileCount; i++) {
            UpdateProjectile(i, deltaTime);
        }

        // Update enemies
        for (int i = 0; i < enemyCount; i++) {
            UpdateEnemy(i, deltaTime);
        }

        BeginDrawing();
        ClearBackground((Color){ 32, 12, 12, 255 });
        BeginMode2D(camera);
        DrawCircleV(player.pos, player.radius, WHITE);
        for (int i = 0; i < 32; i++) {
            DrawCircleV(Vector2Add(player.pos, (Vector2){ 0, -i / 320.0f }), player.radius, WHITE);
        }

        for (int i = 0; i < MAX_SHAPES; i++) {
            DrawShape(shapes[i]);
        }

        // Draw projectiles
        for (int i = 0; i < projectileCount; i++) {
            DrawProjectile(i);
        }

        // Draw enemies
        for (int i = 0; i < enemyCount; i++) {
            DrawEnemy(i);
        }

        EndMode2D();

        DrawCircleV(GetMousePosition(), 2, RED);

        EndDrawing();
    }

    CloseWindow();
}
