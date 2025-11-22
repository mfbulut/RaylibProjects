typedef struct {
    Vector2 pos;
    Vector2 vel;
    Vector2 prevPos;
    float radius;
    float mass;
    Color color;
    bool elastic;
} Ball;

int ballCount = 0;
int shapeCount = 3;
int force = 50;

#define MAX_SHAPES 3
#define MAX_BALLS 256

Shape shapes[MAX_SHAPES] = {
    (Shape){ SHAPE_CIRCLE, {0.0f, -0.5f}, {0.2f, 0}, 0, (Color) { 128, 96, 0, 255 } },
    (Shape){ SHAPE_BOX, {-0.75f, 0.25f}, {0.15f, 0.15f}, 0, (Color) { 128, 96, 0, 255 } },
    (Shape){ SHAPE_ORIENTED_BOX, {0.75f, 0.25f}, {0.1f, 0.25f}, PI / 4, (Color) { 128, 96, 0, 255 } },
};

Ball balls[MAX_BALLS];

void CreateBall(int type) {
    if(ballCount == MAX_BALLS) return;

    balls[ballCount].pos = (Vector2){0, 0};
    balls[ballCount].prevPos = (Vector2){0, 0};
    balls[ballCount].vel = (Vector2){0, 0};
    balls[ballCount].radius = 0.05f;
    balls[ballCount].mass = balls[ballCount].radius * balls[ballCount].radius;

    if(type == 0) {
        balls[ballCount].color = RED;
        balls[ballCount].elastic = true;
    }
    else if(type == 1) {
        balls[ballCount].color = YELLOW;
        balls[ballCount].elastic = false;
    }

    ballCount++;
}

void UpdateSimilation(float deltaTime, Vector2 mousePos) {
    for (int i = 0; i < ballCount; i++) {
        if (IsMouseButtonDown(0) && CheckCollisionPointCircle(mousePos, balls[i].pos, balls[i].radius * force)) {
            Vector2 dir = Vector2Subtract(mousePos, balls[i].pos);
            float dist = Vector2Length(dir);
            if(dist > 0.01f) {
                balls[i].pos = Vector2Add(balls[i].pos, Vector2Scale(dir, 10 * deltaTime));
                balls[i].vel = Vector2Scale(dir, 10);
            }
        }

        balls[i].prevPos = balls[i].pos;
        balls[i].pos = Vector2Add(balls[i].pos, Vector2Scale(balls[i].vel, deltaTime));

        for (int j = 0; j < shapeCount; j++) {
            float newDist = SDF(shapes[j], balls[i].pos);
            if (newDist < balls[i].radius) {
                Vector2 normal = computeNormal(shapes[j], balls[i].pos, 0.01f);
                balls[i].pos = Vector2Add(balls[i].pos, Vector2Scale(normal, balls[i].radius - newDist));
                if(balls[i].elastic)
                    balls[i].vel = Vector2Scale(Vector2Reflect(Vector2Normalize(balls[i].vel), normal), Vector2Length(balls[i].vel));
            }
        }

        float restitution = 0.75f;
        if(balls[i].elastic)
            restitution = 1.0f;

        if(!balls[i].elastic)
            balls[i].vel = Vector2Scale(Vector2Subtract(balls[i].pos, balls[i].prevPos), 1 / deltaTime);

        for (int j = 0; j < ballCount; j++) {
            if (i != j) {
                Vector2 dist = Vector2Subtract(balls[j].pos, balls[i].pos);
                float distance = Vector2Length(dist);
                float minDist = balls[i].radius + balls[j].radius;

                if (distance < minDist) {
                    Vector2 normal = Vector2Normalize(dist);
                    Vector2 relativeVelocity = Vector2Subtract(balls[j].vel, balls[i].vel);
                    float velAlongNormal = Vector2DotProduct(relativeVelocity, normal);

                    float correction = (minDist - distance) / 2.0f;
                    balls[i].pos = Vector2Subtract(balls[i].pos, Vector2Scale(normal, correction));
                    balls[j].pos = Vector2Add(balls[j].pos, Vector2Scale(normal, correction));

                    if (velAlongNormal < 0) {
                        float impulse = -(1 + (1.0f)) * velAlongNormal;
                        impulse /= (1 / balls[i].mass + 1 / balls[j].mass);

                        balls[i].vel = Vector2Add(balls[i].vel, Vector2Scale(normal, -impulse / balls[i].mass));
                        balls[j].vel = Vector2Add(balls[j].vel, Vector2Scale(normal, impulse / balls[j].mass));
                    }
                }
            }
        }
    }
}

void DrawSimilation() {
    for (int i = 0; i < ballCount; i++) {
        DrawCircleV(balls[i].pos, balls[i].radius, balls[i].color);
    }

    for (int i = 0; i < shapeCount; i++) {
        DrawShape(shapes[i]);
    }
}