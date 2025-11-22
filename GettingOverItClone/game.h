Rigidbody rigidBodies[256];
int rigidBodyCount = 0;

Joint joints[32];
int jointCount = 0;

Anchor anchor = { 0 };
Camera2D camera = {(Vector2){ width/2.0f, height/2.0f + 100 }, (Vector2){0, 0}, 0.0f, 0.6f};

Vector2 poly1[] = {(Vector2){1000, 0}, (Vector2){950, -200}, (Vector2){850, -200},(Vector2){800, 0}};
Vector2 poly2[] = {(Vector2){-500, 0}, (Vector2){-600, -200}, (Vector2){-700, 0} };
Vector2 poly3[] = {(Vector2){176,-125}, (Vector2){100,-180}, (Vector2){24,-125}, (Vector2){53,-35}, (Vector2){147,-35}};

// #define DRAW_NORMALS

Texture boxTexture;

void InitGame() {
    boxTexture = LoadTexture("box.png");
    PhysicsMaterial material = { 0.1f, 2.0f };

    rigidBodies[0] = CreateRigidbody(CreatePolygon(poly3, 5), 10, material);  // Player Body
    rigidBodies[1] = CreateRigidbody(CreateRectangle((Vector2){100, -200}, 10, 200), 1, material); // Hammer Body
    rigidBodies[2] = CreateRigidbody(CreateRectangle((Vector2){100, -300}, 80, 30), 1, material);  // Hammer Head

    rigidBodies[3] = CreateRigidbody(CreateRectangle((Vector2){0, 50}, 4000, 100), 0, material);
    rigidBodies[4] = CreateRigidbody(CreateRectangle((Vector2){-300, -100}, 100, 100), 1, material);
    rigidBodies[5] = CreateRigidbody(CreateRectangle((Vector2){-500, -200}, 100, 100), 1, material);
    rigidBodies[6] = CreateRigidbody(CreateRectangle((Vector2){-300, -300}, 100, 100), 1, material);
    rigidBodies[7] = CreateRigidbody(CreateRectangle((Vector2){-500, -400}, 100, 100), 1, material);
    rigidBodies[8] = CreateRigidbody(CreateRectangle((Vector2){-300, -500}, 100, 100), 1, material);

    rigidBodies[9]  = CreateRigidbody(CreatePolygon(poly1, 4), 0, material);
    rigidBodies[10] = CreateRigidbody(CreatePolygon(poly2, 3), 1, material);

    rigidBodyCount = 11;

    joints[0] = CreateJoint(CreateAnchor(&rigidBodies[0], (Vector2){100, -100}), CreateAnchor(&rigidBodies[1], (Vector2){100, -100}), HINGE_JOINT);
    joints[1] = CreateJoint(CreateAnchor(&rigidBodies[1], (Vector2){100, -300}), CreateAnchor(&rigidBodies[2], (Vector2){100, -300}), FIXED_JOINT);
    jointCount = 2;

    // Bridge parameters
    int bridgePlanks = 10;
    float plankWidth = 100;
    float plankHeight = 10;
	Vector2 startPos = { 2050, 50 };

    for (int i = 0; i < bridgePlanks; i++) {
        Vector2 plankPos = { startPos.x + i * plankWidth, startPos.y };
        rigidBodies[rigidBodyCount] = CreateRigidbody(CreateRectangle(plankPos, plankWidth, plankHeight), 5, material);

        if (i > 0) {
            joints[jointCount] = CreateJoint(
                CreateAnchor(&rigidBodies[rigidBodyCount - 1], (Vector2){ plankPos.x - plankWidth / 2, plankPos.y }),
                CreateAnchor(&rigidBodies[rigidBodyCount], (Vector2){ plankPos.x - plankWidth / 2, plankPos.y }),
                FORCE_JOINT
            );
            jointCount++;
        }

        rigidBodyCount++;
    }

    joints[jointCount++] = CreateJoint(CreateAnchor(&rigidBodies[3], startPos), CreateAnchor(&rigidBodies[rigidBodyCount - bridgePlanks], (Vector2){ startPos.x, startPos.y }), FORCE_JOINT);
    joints[jointCount++] = CreateJoint(CreateAnchor(&rigidBodies[3], (Vector2){ startPos.x + (bridgePlanks - 1) * plankWidth, startPos.y }), CreateAnchor(&rigidBodies[rigidBodyCount - 1], (Vector2){ startPos.x + (bridgePlanks - 1) * plankWidth, startPos.y }), FORCE_JOINT);
}

void UpdateGame(float deltaTime) {
	Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

    if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        for (int i = 0; i < rigidBodyCount; i++) {

            if(CheckCollisionPointPoly(mousePos, rigidBodies[i].polygon->vertices, rigidBodies[i].polygon->vertexCount)) {
                anchor = CreateAnchor(&rigidBodies[i], mousePos);
            }
        }
    }

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        anchor.rigidbody = 0;
    }

    if(IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 wishDir = Vector2Normalize(Vector2Subtract(mousePos, rigidBodies[0].polygon->centroid));
        Vector2 currentdir = Vector2Normalize(Vector2Subtract(rigidBodies[1].polygon->centroid, rigidBodies[0].polygon->centroid));

        float angleToMouse = atan2f(wishDir.y, wishDir.x);
        float currentAngle = atan2f(currentdir.y, currentdir.x);

        float angleDifference = angleToMouse - currentAngle;
        if (angleDifference > PI) angleDifference -= 2 * PI;
        if (angleDifference < -PI) angleDifference += 2 * PI;

        rigidBodies[1].angularVelocity *= 0.0f;
        rigidBodies[1].angularVelocity += angleDifference * 2000.0f * deltaTime;
    }

    if(anchor.rigidbody) {
        Vector2 anchorPos = AnchorPosition(anchor);
        Vector2 mouseForce = Vector2Scale(Vector2Subtract(mousePos, anchorPos), 50.0f * anchor.rigidbody->mass);
        ApplyForceAtPoint(anchor.rigidbody, mouseForce, anchorPos);
    }

    const int subStepCount = 5;
    float sdt = deltaTime / subStepCount;

    for (int i = 0; i < subStepCount; i++) {
        for (int i = 0; i < rigidBodyCount; i++) {
            Rigidbody* rb = &rigidBodies[i];
            AddForce(rb, Vector2Scale((Vector2){0, 800}, rb->mass));
            UpdateRigidbody(rb, sdt);
        }

        for (int i = 0; i < jointCount; i++) {
            UpdateJoint(&joints[i], sdt);
        }

        for (int i = 0; i < rigidBodyCount; i++) {
            for (int j = i + 1; j < rigidBodyCount; j++) {
                if((i == 1) || (j == 1)) continue;
                HandleCollision(&rigidBodies[i], &rigidBodies[j]);
            }
        }
    }
}

void DrawGame() {
    camera.target = rigidBodies[0].polygon->centroid;
    BeginMode2D(camera);

    if(anchor.rigidbody) {
        DrawLineV(GetScreenToWorld2D(GetMousePosition(), camera), AnchorPosition(anchor), RED);
        DrawCircleV(AnchorPosition(anchor), 5, RED);
    }

    for (int i = 0; i < jointCount; i++) {
        DrawLineV(AnchorPosition(joints[i].anchorA), AnchorPosition(joints[i].anchorB), BLUE);
        DrawCircleV(AnchorPosition(joints[i].anchorA), 5, BLUE);
        DrawCircleV(AnchorPosition(joints[i].anchorB), 5, BLUE);
    }

    for (int i = 0; i < rigidBodyCount; i++) {
        Polygon* polygon = rigidBodies[i].polygon;
        DrawTriangleFan(polygon->vertices, polygon->vertexCount, DARKGRAY);

        for (int i = 0; i < polygon->vertexCount; i++) {
            DrawLineV(polygon->vertices[i], polygon->vertices[(i + 1) % polygon->vertexCount], WHITE);

            #ifdef DRAW_NORMALS
            Vector2 midpoint = Vector2Scale(Vector2Add(polygon->vertices[i], polygon->vertices[(i + 1) % polygon->vertexCount]), 0.5f);
            Vector2 endpoint = Vector2Add(midpoint, Vector2Scale(polygon->normals[i], 10));
            DrawLineV(midpoint, endpoint, GREEN);
            #endif
        }
    }

    SetShapesTexture(boxTexture, (Rectangle){ 0, 0, boxTexture.width, boxTexture.height});

    for (int i = 4; i < 9; i++) {
        Polygon* polygon = rigidBodies[i].polygon;
        DrawTriangleFan(polygon->vertices, polygon->vertexCount, WHITE);
    }

    SetShapesTexture((Texture2D){0}, (Rectangle){0, 0, 0, 0});

    EndMode2D();
}