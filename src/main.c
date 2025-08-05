#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <raylib.h>
#include <raymath.h>

typedef struct {
    Vector2 pos;
    bool alive;
} Object;

typedef struct {
    Vector2 pos;
    Vector2 dir;
    bool alive;
} Projectile;

// player
Vector2 player = { 0, 0 };
#define PLAYER_SPEED 345

// object
Object *objects = NULL;

#define OBJ_COUNT 456
#define OBJ_RADIUS 7

// projectile
Projectile *projectiles = NULL;

#define PROJ_SPEED 234
#define PROJ_RADIUS 3
#define SPLIT_PROJ_COUNT 3

// functions
void init_obj(int width, int height) {
    objects = malloc(sizeof(Object) * OBJ_COUNT);
    if (objects == NULL) {
        printf("alloc failed\n");

        exit(-1);
    }

    for (int i = 0; i < OBJ_COUNT; i++) {
        objects[i].pos.x = rand() % width;
        objects[i].pos.y = rand() % height;

        objects[i].alive = true;
    }
}

void shoot_projectile(Vector2 from, Vector2 to, int *proj_count) {
    Projectile *temp = realloc(projectiles, sizeof(Projectile) * (*proj_count + 1));
    if (temp == NULL) {
        printf("realloc failed\n");

        free(projectiles);
        exit(-1);
    }
    projectiles = temp;

    projectiles[*proj_count].pos = from;
    projectiles[*proj_count].dir = Vector2Normalize(Vector2Subtract(to, from));
    projectiles[*proj_count].alive = true;

    (*proj_count)++;
}

void update_projectiles(float dt, int proj_count) {
    for (int i = 0; i < proj_count; i++) {
        if (!projectiles[i].alive) continue;
        projectiles[i].pos = Vector2Add(projectiles[i].pos, Vector2Scale(projectiles[i].dir, PROJ_SPEED * dt));

        Vector2 pos = projectiles[i].pos;
        if (pos.x < 0 || pos.x > GetScreenWidth() || pos.y < 0 || pos.y > GetScreenHeight()) {
            projectiles[i].alive = false;
        }
    }
}

void check_coll(int *proj_count, int *obj_count_display) {
    float combined_radius = OBJ_RADIUS + PROJ_RADIUS;

    for (int i = 0; i < *proj_count; i++) {
        if (!projectiles[i].alive) continue;

        for (int j = 0; j < OBJ_COUNT; j++) {
            if (!objects[j].alive) continue;

            float dist = Vector2Distance(projectiles[i].pos, objects[j].pos);
            if (dist < combined_radius) {
                projectiles[i].alive = false;
                objects[j].alive = false;

                (*obj_count_display)--;

                for (int k = 0; k < SPLIT_PROJ_COUNT; k++) {
                    float angle = ((float) rand() / RAND_MAX) * 2 * PI;
                    Vector2 dir = { cosf(angle), sinf(angle) };

                    shoot_projectile(objects[j].pos, Vector2Add(objects[j].pos, dir), proj_count);
                }
                break;
            }
        }
    }
}

void cleanup_projectiles(int *proj_count) {
    int alive_count = 0;

    for (int i = 0; i < *proj_count; i++) {
        if (projectiles[i].alive) projectiles[alive_count++] = projectiles[i];
    }

    if (alive_count != *proj_count) {
        Projectile *temp = realloc(projectiles, sizeof(Projectile) * alive_count);
        if (temp == NULL && alive_count > 0) {
            printf("cleanup realloc failed\n");

            return;
        }

        projectiles = temp;
        *proj_count = alive_count;
    }
}

int main(int argc, char **argv) {
    int obj_count_display = OBJ_COUNT;
    int proj_count = 0;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1600, 800, "public static void main(String[] args)");
    SetTargetFPS(165);

    init_obj(GetRenderWidth(), GetRenderHeight());

    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyDown(KEY_W)) player.y -= PLAYER_SPEED * delta_time;
        if (IsKeyDown(KEY_A)) player.x -= PLAYER_SPEED * delta_time;
        if (IsKeyDown(KEY_S)) player.y += PLAYER_SPEED * delta_time;
        if (IsKeyDown(KEY_D)) player.x += PLAYER_SPEED * delta_time;

        if (IsKeyPressed(KEY_G)) {
            proj_count = 0;
            free(projectiles);
            projectiles = NULL;

            free(objects);
            init_obj(GetRenderWidth(), GetRenderHeight());

            obj_count_display = OBJ_COUNT;
        }

        if (IsKeyDown(KEY_R)) {
            free(objects);
            init_obj(GetRenderWidth(), GetRenderHeight());

            obj_count_display = OBJ_COUNT;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            shoot_projectile(player, GetMousePosition(), &proj_count);
        }

        update_projectiles(delta_time, proj_count);
        check_coll(&proj_count, &obj_count_display);
        cleanup_projectiles(&proj_count);

        BeginDrawing(); {
            ClearBackground(BLACK);

            for (int i = 0; i < OBJ_COUNT; i++) {
                if (objects[i].alive) DrawCircleV(objects[i].pos, OBJ_RADIUS, WHITE);
            }

            DrawCircleV(player, 7, PINK);

            for (int i = 0; i < proj_count; i++) {
                if (projectiles[i].alive) DrawCircleV(projectiles[i].pos, PROJ_RADIUS, RED);
            }

            DrawFPS(10, 10);
            DrawText(TextFormat("Proj count: %d", proj_count), 10, 30, 20, LIME);
            DrawText(TextFormat("Obj count: %d", obj_count_display), 10, 50, 20, LIME);

        } EndDrawing();
    }

    free(projectiles);
    free(objects);
    CloseWindow();
    return 0;
}
