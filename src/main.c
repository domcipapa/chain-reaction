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
float proj_speed = 300;

Vector2 player = { 0, 0 };
int player_speed = 500;

Object *objects = NULL;
int obj_count = 300, obj_count_display;
float obj_radius = 8.0f;

Projectile *projectiles = NULL;
int proj_count = 0;
float proj_radius = 4.0f;
int split_proj_count = 3;

void init_obj(int width, int height) {
    objects = malloc(sizeof(Object) * obj_count);
    if (objects == NULL) {
        printf("alloc fail");
        exit(-1);
    }

    for (int i = 0; i < obj_count; i++) {
        objects[i].pos.x = rand() % width;
        objects[i].pos.y = rand() % height;
        objects[i].alive = true;
    }
}

void shoot_projectile(Vector2 from, Vector2 to) {
    Projectile *temp = realloc(projectiles, sizeof(Projectile) * (proj_count + 1));
    if (temp == NULL) {
        printf("realloc fail");
        free(projectiles);
        exit(-1);
    }
    projectiles = temp;

    projectiles[proj_count].pos = from;
    projectiles[proj_count].dir = Vector2Normalize(Vector2Subtract(to, from));
    projectiles[proj_count].alive = true;

    proj_count++;
}

void update_projectiles(float dt) {
    for (int i = 0; i < proj_count; i++) {
        if (!projectiles[i].alive) continue;
        projectiles[i].pos = Vector2Add(projectiles[i].pos, Vector2Scale(projectiles[i].dir, proj_speed * dt));

        Vector2 pos = projectiles[i].pos;
        if (pos.x < 0 || pos.x > GetScreenWidth() || pos.y < 0 || pos.y > GetScreenHeight()) {
            projectiles[i].alive = false;
        }
    }
}

void check_coll() {
    float combined_radius = obj_radius + proj_radius;

    for (int i = 0; i < proj_count; i++) {
        if (!projectiles[i].alive) continue;

        for (int j = 0; j < obj_count; j++) {
            if (!objects[j].alive) continue;

            float dist = Vector2Distance(projectiles[i].pos, objects[j].pos);
            if (dist < combined_radius) {
                projectiles[i].alive = false;
                objects[j].alive = false;
                obj_count_display--;

                for (int k = 0; k < split_proj_count; k++) {
                    float angle = ((float) rand() / RAND_MAX) * 2 * PI;
                    Vector2 dir = { cosf(angle), sinf(angle) };
                    shoot_projectile(objects[j].pos, Vector2Add(objects[j].pos, dir));
                }
                break;
            }
        }
    }
}

void cleanup_projectiles() {
    int alive_count = 0;

    for (int i = 0; i < proj_count; i++) {
        if (projectiles[i].alive) projectiles[alive_count++] = projectiles[i];
    }

    if (alive_count != proj_count) {
        Projectile *temp = realloc(projectiles, sizeof(Projectile) * alive_count);
        if (temp == NULL && alive_count > 0) {
            printf("Cleanup realloc failed\n");
            return;
        }

        projectiles = temp;
        proj_count = alive_count;
    }
}

int main(int argc, char **argv) {
    obj_count_display = obj_count;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1600, 800, "public static void main(String[] args)");
    SetTargetFPS(165);

    init_obj(GetRenderWidth(), GetRenderHeight());
    while (!WindowShouldClose()) {
        float delta_time = GetFrameTime();

        if (IsKeyDown(KEY_W)) player.y -= player_speed * delta_time;
        if (IsKeyDown(KEY_A)) player.x -= player_speed * delta_time;
        if (IsKeyDown(KEY_S)) player.y += player_speed * delta_time;
        if (IsKeyDown(KEY_D)) player.x += player_speed * delta_time;

        if (IsKeyPressed(KEY_G)) {
            proj_count = 0;
            free(projectiles);
            projectiles = NULL;

            free(objects);
            init_obj(GetRenderWidth(), GetRenderHeight());
            obj_count_display = obj_count;
        }

        if (IsKeyDown(KEY_R)) {
            free(objects);

            init_obj(GetRenderWidth(), GetRenderHeight());
            obj_count_display = obj_count;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            shoot_projectile(player, GetMousePosition());
        }
        update_projectiles(delta_time);

        check_coll();
        cleanup_projectiles();

        BeginDrawing(); {

            ClearBackground(BLACK);

            for (int i = 0; i < obj_count; i++) {
                if (objects[i].alive) DrawCircleV(objects[i].pos, obj_radius, WHITE);
            }

            DrawCircleV(player, 12, PINK);

            for (int i = 0; i < proj_count; i++) {
                if (projectiles[i].alive) DrawCircleV(projectiles[i].pos, proj_radius, RED);
            }

            DrawFPS(10, 10);
            DrawText(TextFormat("Proj count: %d", proj_count), 10, 30, 20, LIME);
            DrawText(TextFormat("Obj count: %d", obj_count_display), 10, 50, 20, LIME);

        } EndDrawing();
    }
    return 0;
}
