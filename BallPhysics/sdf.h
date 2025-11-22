typedef enum {
    SHAPE_CIRCLE,
    SHAPE_BOX,
    SHAPE_ORIENTED_BOX
} ShapeType;

typedef struct {
    ShapeType type;
    Vector2 pos;
    Vector2 size; // size.x is also radius for circle
    float angle;
    Color color;
} Shape;

Vector2 Vector2Abs(Vector2 v) {
    return (Vector2){ fabsf(v.x), fabsf(v.y) };
}

float SDF_Circle(Vector2 point, Vector2 pos, float radius) {
    return Vector2Distance(point, pos) - radius;
}

float SDF_Box(Vector2 point, Vector2 pos, Vector2 size) {
    Vector2 d = Vector2Subtract(Vector2Abs(Vector2Subtract(point, pos)), size);
    return Vector2Length(Vector2Max(d, (Vector2){0, 0})) + fminf(fmaxf(d.x, d.y), 0.0);
}

float SDF_OrientedBox(Vector2 point, Vector2 pos, Vector2 size, float angle) {
    Vector2 rPos = Vector2Subtract(point, pos);
    Vector2 rotatedPos = {
        cosf(angle) * rPos.x + sinf(angle) * rPos.y,
        -sinf(angle) * rPos.x + cosf(angle) * rPos.y
    };
    Vector2 d = Vector2Subtract(Vector2Abs(rotatedPos), size);
    return Vector2Length(Vector2Max(d, (Vector2){0, 0})) + fminf(fmaxf(d.x, d.y), 0.0);
}

float SDF(Shape shape, Vector2 point) {
    switch (shape.type) {
        case SHAPE_CIRCLE:
            return SDF_Circle(point, shape.pos, shape.size.x);
        case SHAPE_BOX:
            return SDF_Box(point, shape.pos, shape.size);
        case SHAPE_ORIENTED_BOX:
            return SDF_OrientedBox(point, shape.pos, shape.size, shape.angle);
        default:
            return 0.0f;
    }
}

Vector2 computeNormal(Shape shape, Vector2 point, float epsilon) {
    Vector2 normal = {
        SDF(shape, (Vector2){point.x + epsilon, point.y}) - SDF(shape, (Vector2){point.x - epsilon, point.y}),
        SDF(shape, (Vector2){point.x, point.y + epsilon}) - SDF(shape, (Vector2){point.x, point.y - epsilon})
    };
    // Normalize the vector
    float length = sqrtf(normal.x * normal.x + normal.y * normal.y);
    if (length > 0) {
        normal.x /= length;
        normal.y /= length;
    }
    return normal;
}

void DrawShape(Shape shape) {
    switch (shape.type) {
        case SHAPE_CIRCLE:
            DrawCircleV(shape.pos, shape.size.x, shape.color);
            break;
        case SHAPE_BOX:
            DrawRectangleV(Vector2Subtract(shape.pos, shape.size), Vector2Scale(shape.size, 2.0f), shape.color);
            break;
        case SHAPE_ORIENTED_BOX:
            Rectangle rec = { shape.pos.x, shape.pos.y, shape.size.x * 2.0f, shape.size.y * 2.0f };
            DrawRectanglePro(rec, (Vector2){shape.size.x, shape.size.y}, shape.angle * RAD2DEG, shape.color);
            break;
    }
}