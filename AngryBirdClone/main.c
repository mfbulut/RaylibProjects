#include "raylib.h"
#include "raymath.h"

#define FPHYSICS_IMPLEMENTATION
#include "fphysics.h"
#include "joint.h"

const int width = 1280, height = 720;

Rigidbody rigidBodies[1024];
int rigidBodyCount = 0;

Joint joints[1024];
int jointCount = 0;

bool slingshotStarted = false;
Rigidbody* currentBird = 0;
int birdCount = 0;

Vector2 lastSlingshotStart = {0};
Vector2 lastSlingshotEnd = {0};
bool showLastSlingshot = false;

const float MAX_SLINGSHOT_DISTANCE = 150.0f;
const float SLINGSHOT_LINE_THICKNESS = 4.0f;

Polygon* CreateCircle(Vector2 position, float radius) {
    const int vertexCount = 20;
    Vector2* vertices = (Vector2*)MemAlloc(vertexCount * sizeof(Vector2));
    for (int i = 0; i < vertexCount; i++) {
        float angle = 2 * PI * i / vertexCount;
        vertices[i] = (Vector2){ position.x - radius * cosf(angle), position.y + radius * sinf(angle) };
    }
    return CreatePolygon(vertices, vertexCount);
    MemFree(vertices);
}

typedef struct {
    const char* layout;
    int width;
    int height;
} Level;

const Level LEVELS[] = {
    {
        "...................."
        "...................."
        "...................."
        "...########........."
        "..##.#....#........."
        ".##..#....#........."
        ".#...#....#......#.."
        ".#...#....#......#.."
        "##...#....##....##.."
        "##################..",
        20, 10
    },
};

void AddJointBetweenBodies(Rigidbody* rb1, Rigidbody* rb2, Vector2 connectionPoint, JointType type) {
    if (jointCount >= 1024) return;
    joints[jointCount] = CreateJoint(
        CreateAnchor(rb1, connectionPoint),
        CreateAnchor(rb2, connectionPoint),
        type
    );
    jointCount++;
}

void LoadLevel(int levelIndex) {
    rigidBodyCount = 0;
    jointCount = 0;
    const Level* level = &LEVELS[levelIndex];
    PhysicsMaterial material = {0.1f, 0.05f};

    Rigidbody* ground = &rigidBodies[rigidBodyCount++];
    *ground = CreateRigidbody(CreateRectangle((Vector2){0, 700}, 4000, 100), 0, material);

    float blockSize = 30.0f;
    float startX = 100.0f;
    float startY = 365.0f;

    Rigidbody* gridBodies[64][64] = {0};
    int index = 0;
    for (int y = 0; y < level->height; y++) {
        for (int x = 0; x < level->width; x++) {
            char cell = level->layout[index++];
            Vector2 position = {
                startX + x * blockSize,
                startY + y * blockSize
            };
            switch (cell) {
                case '#': {
                    rigidBodies[rigidBodyCount] = CreateRigidbody(
                        CreateRectangle(position, blockSize - 3, blockSize - 3),
                        1,
                        material
                    );
                    gridBodies[y][x] = &rigidBodies[rigidBodyCount];
                    rigidBodyCount++;
                    break;
                }
                case 'O': {
                    rigidBodies[rigidBodyCount] = CreateRigidbody(
                        CreateCircle(position, 15),
                        1,
                        material
                    );
                    rigidBodyCount++;
                    break;
                }
            }
        }
    }


    for (int y = 0; y < level->height; y++) {
        for (int x = 0; x < level->width; x++) {
            if (!gridBodies[y][x]) continue;
            Vector2 center = gridBodies[y][x]->polygon->centroid;

            if (x < level->width - 1 && gridBodies[y][x + 1]) {
                Vector2 connectionPointUp = {
                    center.x + blockSize/2,
                    center.y - blockSize/2
                };

                Vector2 connectionPointDown = {
                    center.x + blockSize/2,
                    center.y + blockSize/2
                };

                AddJointBetweenBodies(gridBodies[y][x], gridBodies[y][x + 1], connectionPointUp, FORCE_JOINT);
                AddJointBetweenBodies(gridBodies[y][x], gridBodies[y][x + 1], connectionPointDown, FORCE_JOINT);
            }

            if (y < level->height - 1 && gridBodies[y + 1][x]) {
                Vector2 connectionPointRight = {
                    center.x + blockSize/2,
                    center.y + blockSize/2
                };
                Vector2 connectionPoint2Left = {
                    center.x - blockSize/2,
                    center.y + blockSize/2
                };
                AddJointBetweenBodies(gridBodies[y][x], gridBodies[y + 1][x], connectionPointRight, FORCE_JOINT);
                AddJointBetweenBodies(gridBodies[y][x], gridBodies[y + 1][x], connectionPoint2Left, FORCE_JOINT);
            }


            if (y == level->height - 1 && gridBodies[y][x]) {
                Vector2 connectionPoint = {
                    center.x,
                    center.y + blockSize/2
                };
                AddJointBetweenBodies(gridBodies[y][x], ground, connectionPoint, HINGE_JOINT);
            }
        }
    }
}

void InitGame() {
    PhysicsMaterial material = {0.1f, 0.05f};
    LoadLevel(0);

    for (int i = 0; i < 3; i++) {
        rigidBodies[rigidBodyCount++] = CreateRigidbody(CreateCircle((Vector2){1000, 500}, 10), 0, material);
        rigidBodies[rigidBodyCount - 1].id = 2;
    }

    currentBird = &rigidBodies[rigidBodyCount - 1];
}

void UpdateGame(float deltaTime) {
    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (currentBird != 0 && CheckCollisionPointCircle(mousePos, currentBird->polygon->centroid, 10)) {
            slingshotStarted = true;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (slingshotStarted && currentBird != 0) {
            Vector2 direction = Vector2Subtract(currentBird->polygon->centroid, GetMousePosition());
            float distance = Vector2Length(direction);

            if (distance > MAX_SLINGSHOT_DISTANCE) {
                direction = Vector2Scale(Vector2Normalize(direction), MAX_SLINGSHOT_DISTANCE);
            }
            Vector2 releaseVelocity = Vector2Scale(direction, 6.0f);

            currentBird->isKinematic = 0;
            currentBird->mass = 4;
            currentBird->invMass = 1.0f / currentBird->mass;
            currentBird->velocity = releaseVelocity;
            slingshotStarted = false;


            lastSlingshotStart = currentBird->polygon->centroid;
            lastSlingshotEnd = Vector2Subtract(currentBird->polygon->centroid, direction);
            showLastSlingshot = true;

            currentBird->id = 1;
            currentBird = 0;

            for (int i = 0; i < rigidBodyCount; i++) {
                if (rigidBodies[i].id == 2) {
                    currentBird = &rigidBodies[i];
                }
            }
        }
    }

    const int subStepCount = 10;
    float sdt = deltaTime / subStepCount;

    for (int i = 0; i < subStepCount; i++) {
        for (int i = 0; i < rigidBodyCount; i++) {
            Rigidbody* rb = &rigidBodies[i];
            AddForce(rb, Vector2Scale((Vector2){0, 1200}, rb->mass));
            UpdateRigidbody(rb, sdt);
        }

        for (int i = 0; i < jointCount; i++) {
            UpdateJoint(&joints[i], sdt);
        }

        for (int i = 0; i < rigidBodyCount; i++) {
            for (int j = i + 1; j < rigidBodyCount; j++) {
                if(rigidBodies[i].id != 2)
                    HandleCollision(&rigidBodies[i], &rigidBodies[j]);
            }
        }
    }
}

void DrawGame() {
    for (int i = 0; i < jointCount; i++) {
        if(joints[i].type != EMPTY_JOINT){
            DrawLineV(AnchorPosition(joints[i].anchorA), AnchorPosition(joints[i].anchorB), BLUE);
            DrawCircleV(AnchorPosition(joints[i].anchorA), 5, BLUE);
            DrawCircleV(AnchorPosition(joints[i].anchorB), 5, BLUE);
        }
    }

    for (int i = 0; i < rigidBodyCount; i++) {
        Polygon* polygon = rigidBodies[i].polygon;
        DrawTriangleFan(polygon->vertices, polygon->vertexCount, DARKGRAY);

        for (int i = 0; i < polygon->vertexCount; i++) {
            DrawLineV(polygon->vertices[i], polygon->vertices[(i + 1) % polygon->vertexCount], WHITE);
        }
    }

if (slingshotStarted && currentBird) {
        Vector2 slingshotDir = Vector2Subtract(currentBird->polygon->centroid, GetMousePosition());
        float distance = Vector2Length(slingshotDir);

        Vector2 clampedMousePos = GetMousePosition();
        if (distance > MAX_SLINGSHOT_DISTANCE) {
            Vector2 normalized = Vector2Normalize(slingshotDir);
            clampedMousePos = Vector2Subtract(currentBird->polygon->centroid,
                Vector2Scale(normalized, MAX_SLINGSHOT_DISTANCE));
        }

        float stretchPercentage = fminf(distance / MAX_SLINGSHOT_DISTANCE, 1.0f);
        Color slingshotColor = ColorLerp(GREEN, RED, stretchPercentage);

        DrawLineEx(
            clampedMousePos,
            currentBird->polygon->centroid,
            SLINGSHOT_LINE_THICKNESS,
            slingshotColor
        );
    }





    if (currentBird && showLastSlingshot) {
        DrawLineEx(
            lastSlingshotStart,
            lastSlingshotEnd,
            SLINGSHOT_LINE_THICKNESS,
            Fade(LIGHTGRAY, 0.2f)
        );
    }

}

int main(void) {
    SetConfigFlags(FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(width, height, "Angry Birds Clone");
    InitGame();

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        UpdateGame(deltaTime);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawGame();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}