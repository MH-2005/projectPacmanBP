#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "raylib.h"
#include "raymath.h"

#ifndef TILE_SIZE
#define TILE_SIZE 20
#endif

#define MAZE_WIDTH 28
#define MAZE_HEIGHT 31
#define GHOST_COUNT 5
#define GHOST_SPEED 2.0f
#define GHOST_SCATTER_DURATION 7.0f
#define GHOST_CHASE_DURATION 20.0f
#define MIN_GHOST_DISTANCE 1

typedef enum {
    GHOST_STATE_SCATTER,
    GHOST_STATE_CHASE,
    GHOST_STATE_FRIGHTENED
} GhostState;

typedef struct {
    Vector2 position;
    Vector2 target;
    Vector2 direction;
    float speed;
    GhostState state;
    float stateTimer;
    Color color;
    bool isEaten;
    int scatterCount;
} Ghost;

typedef struct {
    Ghost ghosts[GHOST_COUNT];
    float scatterTimer;
    float chaseTimer;
    bool isScatterMode;
} GhostManager;

typedef struct {
    char color[10];
    char direction;
    int x;
} pos;

Vector2 getGhostSpawnPosition(int ghostIndex);
Vector2 getScatterTarget(int ghostIndex);
Vector2 calculateChaseTarget(int index, Vector2 pacmanPos, Ghost* ghosts);
Vector2 getRandomTarget(void);

char initialmaze_G[MAZE_HEIGHT][MAZE_WIDTH] = {
        "############################",
        "#            ##            #",
        "# ###### ### ## ### ###### #",
        "# ###### ### ## ### ###### #",
        "#      #     ##     #      #",
        "### ## # ### ## ### # ## ###",
        "### ## # ### ## ### # ## ###",
        "#   ##                ##   #",
        "# #### ### ###### ### #### #",
        "# #### ### ###### ### #### #",
        "#                          #",
        "### ## ## ###__### ## ## ###",
        "### ## ## #RRRRRR# ## ## ###",
        "#         #RRRRRR#         #",
        "# #### ## #RRRRRR# ## #### #",
        "# #### ## ######## ## #### #",
        "#    #                #    #",
        "#### # ## ######## ## # ####",
        "#### # ## ######## ## # ####",
        "#      ##    ##    ##      #",
        "# ########## ## ########## #",
        "# ########## ## ########## #",
        "#    ##              ##    #",
        "#### ## #### ## #### ## ####",
        "#### ## #### ## #### ## ####",
        "#         ##    ##         #",
        "# ### ### ######## ### ### #",
        "# ### ### ######## ### ### #",
        "#                          #",
        "############################"
};

const pos z[20] = {
        {"blue", 'D', 0}, {"blue", 'L', 1}, {"blue", 'R', 2}, {"blue", 'U', 3},
        {"green", 'D', 4}, {"green", 'L', 5}, {"green", 'R', 6}, {"green", 'U', 7},
        {"orange", 'D', 8}, {"orange", 'L', 9}, {"orange", 'R', 10}, {"orange", 'U', 11},
        {"pink", 'D', 12}, {"pink", 'L', 13}, {"pink", 'R', 14}, {"pink", 'U', 15},
        {"red", 'D', 16}, {"red", 'L', 17}, {"red", 'R', 18}, {"red", 'U', 19}
};

int i_pos[GHOST_COUNT], j_pos[GHOST_COUNT];
int last_direction[GHOST_COUNT] = {0};

float get_distance(int x1, int y1, int x2, int y2) {
    int dx = x1 - x2;
    int dy = y1 - y2;
    return (float) sqrt(dx * dx + dy * dy);
}

bool is_position_safe(int ghost_index, int new_i, int new_j) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        if (i != ghost_index) {
            if (get_distance(new_i, new_j, i_pos[i], j_pos[i]) < MIN_GHOST_DISTANCE) {
                return false;
            }
        }
    }
    return true;
}

void ghost_show(pos g, Texture2D ghost[40], int ii, int jj, int ii_p, int jj_p, int *h) {
    static float last_hit_time = 0.0f;
    float current_time = GetTime();

    for (int i = 0; i < 20; i++) {
        if (!strcmp(g.color, z[i].color) && g.direction == z[i].direction) {
            static float timer = 0.0f;
            static int currentFrame = 0;

            if (ii == ii_p && jj == jj_p) {
                if (current_time - last_hit_time >= 1.0f) {
                    (*h)--;
                    last_hit_time = current_time;
                }
            }

            timer += GetFrameTime();
            if (timer >= 0.8f) {
                timer = 0.0f;
                currentFrame = !currentFrame;
            }

            DrawTexture(ghost[z[i].x * 2 + currentFrame], jj * 20 + 70, ii * 20 + 70, WHITE);
            break;
        }
    }
}

void random_place_target(int ghost_index, int *target_row, int *target_col, int ii_p, int jj_p) {
    int available_positions[MAZE_HEIGHT * MAZE_WIDTH][2];
    int count = 0;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (initialmaze_G[i][j] == ' ' &&
                get_distance(i, j, ii_p, jj_p) >= MIN_GHOST_DISTANCE &&
                is_position_safe(ghost_index, i, j)) {
                available_positions[count][0] = i;
                available_positions[count][1] = j;
                count++;
            }
            // خط reset حذف شد: initialmaze_G[i][j] = initialmaze_G_I[i][j];
        }
    }

    if (count == 0) {
        *target_row = -1;
        *target_col = -1;
        return;
    }

    int random_index = rand() % count;
    *target_row = available_positions[random_index][0];
    *target_col = available_positions[random_index][1];
}

typedef struct {
    int i, j;
    char *path;
} QueueElement;

void FindPathUsingBFS(int ghost_index, int target_row, int target_col, int now_row, int now_col, char *path) {
    QueueElement *queue = malloc(sizeof(QueueElement) * MAZE_HEIGHT * MAZE_WIDTH);
    int front = 0, rear = 0;

    queue[rear].i = now_row;
    queue[rear].j = now_col;
    queue[rear].path = malloc(1);
    queue[rear].path[0] = '\0';
    rear++;

    bool visited[MAZE_HEIGHT][MAZE_WIDTH] = {0};
    visited[now_row][now_col] = true;

    while (front < rear) {
        // FIX 6: memory leak رفع شد
        //         قبلا فقط queue آزاد می‌شد ولی path های داخلش leak می‌شدن
        //         الان هر path بعد از پردازش آزاد می‌شه
        int ci = queue[front].i;
        int cj = queue[front].j;
        char *cpath = queue[front].path;
        front++;

        if (ci == target_row && cj == target_col) {
            strcpy(path, cpath);
            free(cpath);
            // آزاد کردن path های باقیمانده در صف
            for (int k = front; k < rear; k++) free(queue[k].path);
            free(queue);
            return;
        }

        const int dr[] = {1, -1, 0, 0};
        const int dc[] = {0, 0, 1, -1};
        const char dir[] = {'D', 'U', 'R', 'L'};

        for (int i = 0; i < 4; i++) {
            int ni = ci + dr[i];
            int nj = cj + dc[i];
            if (ni >= 0 && ni < MAZE_HEIGHT && nj >= 0 && nj < MAZE_WIDTH &&
                initialmaze_G[ni][nj] != '#' && !visited[ni][nj] &&
                is_position_safe(ghost_index, ni, nj)) {
                int clen = strlen(cpath);
                char *new_path = malloc(clen + 2);
                strcpy(new_path, cpath);
                new_path[clen] = dir[i];
                new_path[clen + 1] = '\0';

                queue[rear].i = ni;
                queue[rear].j = nj;
                queue[rear].path = new_path;
                rear++;
                visited[ni][nj] = true;
            }
        }
        free(cpath);  // آزاد کردن path نود فعلی بعد از پردازش
    }

    path[0] = '\0';
    free(queue);
}

void RandomGhostPosition() {
    char initialmaze_G_I[MAZE_HEIGHT][MAZE_WIDTH] = {
            "############################",
            "#            ##            #",
            "# ###### ### ## ### ###### #",
            "# ###### ### ## ### ###### #",
            "#      #     ##     #      #",
            "### ## # ### ## ### # ## ###",
            "### ## # ### ## ### # ## ###",
            "#   ##                ##   #",
            "# #### ### ###### ### #### #",
            "# #### ### ###### ### #### #",
            "#                          #",
            "### ## ## ###__### ## ## ###",
            "### ## ## #RRRRRR# ## ## ###",
            "#         #RRRRRR#         #",
            "# #### ## #RRRRRR# ## #### #",
            "# #### ## ######## ## #### #",
            "#    #                #    #",
            "#### # ## ######## ## # ####",
            "#### # ## ######## ## # ####",
            "#      ##    ##    ##      #",
            "# ########## ## ########## #",
            "# ########## ## ########## #",
            "#    ##              ##    #",
            "#### ## #### ## #### ## ####",
            "#### ## #### ## #### ## ####",
            "#         ##    ##         #",
            "# ### ### ######## ### ### #",
            "# ### ### ######## ### ### #",
            "#                          #",
            "############################"
    };

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            initialmaze_G[i][j] = initialmaze_G_I[i][j];
        }
    }

    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int) time(NULL));
        seeded = true;
    }

    for (int m = 0; m < GHOST_COUNT; m++) {
        int available_positions[MAZE_HEIGHT * MAZE_WIDTH][2];
        int count = 0;

        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                if (initialmaze_G[i][j] == 'R' && is_position_safe(m, i, j)) {
                    available_positions[count][0] = i;
                    available_positions[count][1] = j;
                    count++;
                }
            }
        }

        if (count > 0) {
            int random_index = rand() % count;
            i_pos[m] = available_positions[random_index][0];
            j_pos[m] = available_positions[random_index][1];
            initialmaze_G[i_pos[m]][j_pos[m]] = 'r';
        }
    }
}

int get_direction_index(char direction) {
    switch (direction) {
        case 'D': return 0;
        case 'U': return 1;
        case 'R': return 2;
        case 'L': return 3;
        default: return 0;
    }
}

void movementGhost(int ghost_index, int *j, char path_i[400], bool *found) {
    static float timer = 0.0f;
    static bool hasStarted = false;
    static int pathIndex[GHOST_COUNT] = {0};

    if (!hasStarted && (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))) {
        hasStarted = true;
    }

    if (!hasStarted) return;

    timer += GetFrameTime();
    if (timer < 0.2f) return;
    timer = 0.0f;

    if (!path_i[pathIndex[ghost_index]]) {
        pathIndex[ghost_index] = 0;
        *found = false;
    }

    char current_direction = path_i[pathIndex[ghost_index]];
    *j = get_direction_index(current_direction);
    last_direction[ghost_index] = *j;

    int new_i = i_pos[ghost_index];
    int new_j = j_pos[ghost_index];

    bool can_move = false;
    switch (current_direction) {
        case 'R':
            if (new_j + 1 < MAZE_WIDTH && initialmaze_G[new_i][new_j + 1] != '#') {
                new_j++;
                can_move = true;
            }
            break;
        case 'L':
            if (new_j - 1 >= 0 && initialmaze_G[new_i][new_j - 1] != '#') {
                new_j--;
                can_move = true;
            }
            break;
        case 'U':
            if (new_i - 1 >= 0 && initialmaze_G[new_i - 1][new_j] != '#') {
                new_i--;
                can_move = true;
            }
            break;
        case 'D':
            if (new_i + 1 < MAZE_HEIGHT && initialmaze_G[new_i + 1][new_j] != '#') {
                new_i++;
                can_move = true;
            }
            break;
    }

    if (can_move && is_position_safe(ghost_index, new_i, new_j)) {
        i_pos[ghost_index] = new_i;
        j_pos[ghost_index] = new_j;
        pathIndex[ghost_index]++;
    } else {
        pathIndex[ghost_index] = 0;
        *found = false;
    }
}

void insert_ghost(Texture2D ghost[40], int ii, int jj, int *h) {
    static char path[GHOST_COUNT][400];
    static bool found[GHOST_COUNT] = {false};
    const char *Colors[GHOST_COUNT] = {"blue", "green", "orange", "pink", "red"};
    const char Directions[4] = {'D', 'U', 'R', 'L'};

    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int) time(NULL));
        seeded = true;
    }

    for (int i = 0; i < GHOST_COUNT; i++) {
        if (!found[i]) {
            int chaser1 = rand() % GHOST_COUNT;
            int chaser2 = rand() % GHOST_COUNT;
            while (chaser2 == chaser1) {
                chaser2 = rand() % GHOST_COUNT;
            }
            int target_row, target_col;
            if (i == chaser1 || i == chaser2) {
                target_row = ii;
                target_col = jj;
            } else {
                random_place_target(i, &target_row, &target_col, ii, jj);
            }
            if (target_row != -1 && target_col != -1) {
                FindPathUsingBFS(i, target_row, target_col, i_pos[i], j_pos[i], path[i]);
                if (path[i][0] != '\0') {
                    found[i] = true;
                }
            }
        }

        int direction_index = last_direction[i];
        pos g = {.direction = Directions[direction_index]};
        strcpy(g.color, Colors[i]);

        movementGhost(i, &direction_index, path[i], &found[i]);
        ghost_show(g, ghost, i_pos[i], j_pos[i], ii, jj, h);
    }
}

void GHOST_S_E(Texture2D ghost[4], int index, int *ii, int *jj, int ii_p, int jj_p, int *star, const int *wait_g) {
    static float timer = 0.0f;
    static int c_w = 2;
    static float respawn_timer[GHOST_COUNT];
    static bool is_respawning[GHOST_COUNT] = {false};
    float current_time = GetTime();
    static int frames = 0;
    timer += GetFrameTime();

    if (*wait_g >= 2) {
        c_w = 2;
    } else {
        c_w = 4;
    }

    if (timer >= 0.8f) {
        timer = 0.0f;
        frames = (frames + 1) % c_w;
    }

    if (*ii == ii_p && *jj == jj_p) {
        if (!is_respawning[index] && current_time - respawn_timer[index] >= 1.0f) {
            (*star) += 2;
            is_respawning[index] = true;
            respawn_timer[index] = current_time;
        }
    }
    if (!is_respawning[index] || current_time >= respawn_timer[index] + 1.0f) {
        if (is_respawning[index] && current_time >= respawn_timer[index] + 1.0f) {
            int target_row, target_col;
            random_place_target(index, &target_row, &target_col, ii_p, jj_p);
            *ii = target_row;
            *jj = target_col;
            is_respawning[index] = false;
        }
        DrawTexture(ghost[frames], *jj * 20 + 70, *ii * 20 + 70, WHITE);
    }
}

void eat_ghost(Texture2D ghost[4], int ii, int jj, int *star, const int *wait_g) {
    static char path[GHOST_COUNT][400];
    static bool found[GHOST_COUNT] = {false};
    const char Directions[4] = {'D', 'U', 'R', 'L'};
    for (int i = 0; i < GHOST_COUNT; i++) {
        if (!found[i]) {
            int target_row, target_col;
            random_place_target(i, &target_row, &target_col, ii, jj);
            if (target_row != -1 && target_col != -1) {
                FindPathUsingBFS(i, target_row, target_col, i_pos[i], j_pos[i], path[i]);
                if (path[i][0] != '\0') {
                    found[i] = true;
                }
            }
        }
        int direction_index = last_direction[i];
        movementGhost(i, &direction_index, path[i], &found[i]);
        GHOST_S_E(ghost, i, &i_pos[i], &j_pos[i], ii, jj, star, wait_g);
    }
}

void initGhostManager(GhostManager *manager) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        manager->ghosts[i].position = getGhostSpawnPosition(i);
        manager->ghosts[i].state = GHOST_STATE_CHASE;
        manager->ghosts[i].isEaten = false;
        manager->ghosts[i].speed = GHOST_SPEED;
        manager->ghosts[i].color = (Color){255, 0, 0, 255};
    }
}

void updateGhostStates(GhostManager *manager, float deltaTime) {
    manager->scatterTimer -= deltaTime;
    manager->chaseTimer -= deltaTime;

    if (manager->isScatterMode && manager->scatterTimer <= 0) {
        manager->isScatterMode = false;
        manager->chaseTimer = GHOST_CHASE_DURATION;
        for (int i = 0; i < GHOST_COUNT; i++) {
            manager->ghosts[i].state = GHOST_STATE_CHASE;
        }
    } else if (!manager->isScatterMode && manager->chaseTimer <= 0) {
        manager->isScatterMode = true;
        manager->scatterTimer = GHOST_SCATTER_DURATION;
        for (int i = 0; i < GHOST_COUNT; i++) {
            manager->ghosts[i].state = GHOST_STATE_SCATTER;
        }
    }
}

void updateGhosts(GhostManager *manager, Vector2 pacmanPos, float deltaTime) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *ghost = &manager->ghosts[i];

        if (ghost->isEaten) {
            ghost->position = getGhostSpawnPosition(i);
            ghost->isEaten = false;
            continue;
        }

        switch (ghost->state) {
            case GHOST_STATE_SCATTER:
                ghost->target = getScatterTarget(i);
                break;
            case GHOST_STATE_CHASE:
                ghost->target = calculateChaseTarget(i, pacmanPos, manager->ghosts);
                break;
            case GHOST_STATE_FRIGHTENED:
                ghost->target = getRandomTarget();
                break;
        }

        Vector2 direction = Vector2Normalize(Vector2Subtract(ghost->target, ghost->position));
        ghost->position = Vector2Add(ghost->position,
                                     Vector2Scale(direction, ghost->speed * deltaTime));
    }
}

void drawGhosts(GhostManager *manager) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *ghost = &manager->ghosts[i];
        if (!ghost->isEaten) {
            DrawCircle(
                    (int)ghost->position.x,
                    (int)ghost->position.y,
                    10,
                    ghost->state == GHOST_STATE_FRIGHTENED ? BLUE : ghost->color
            );
        }
    }
}

bool checkGhostCollisions(GhostManager *manager, Vector2 pacmanPos) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *ghost = &manager->ghosts[i];
        if (ghost->isEaten) continue;

        float dx = pacmanPos.x - ghost->position.x;
        float dy = pacmanPos.y - ghost->position.y;
        float distance = sqrt(dx * dx + dy * dy);

        if (distance < 20) {
            return true;
        }
    }
    return false;
}


Vector2 getGhostSpawnPosition(int ghostIndex) {
    Vector2 positions[] = {
            {5 * TILE_SIZE, 5 * TILE_SIZE},
            {22 * TILE_SIZE, 5 * TILE_SIZE},
            {5 * TILE_SIZE, 25 * TILE_SIZE},
            {22 * TILE_SIZE, 25 * TILE_SIZE}
    };
    return positions[ghostIndex % 4];
}

Vector2 getScatterTarget(int ghostIndex) {
    Vector2 corners[] = {
            {0, 0},
            {MAZE_WIDTH * TILE_SIZE, 0},
            {MAZE_WIDTH * TILE_SIZE, MAZE_HEIGHT * TILE_SIZE},
            {0, MAZE_HEIGHT * TILE_SIZE}
    };
    return corners[ghostIndex % 4];
}

Vector2 calculateChaseTarget(int index, Vector2 pacmanPos, Ghost* ghosts) {
    switch (index) {
        case 0:
            return pacmanPos;
        case 1:
            return Vector2Add(pacmanPos, (Vector2){TILE_SIZE * 4, 0});
        case 2: {
            Vector2 blinkyPos = ghosts[0].position;
            Vector2 offset = Vector2Subtract(pacmanPos, blinkyPos);
            return Vector2Add(pacmanPos, offset);
        }
        default:
            return getRandomTarget();
    }
}

Vector2 getRandomTarget(void) {
    return (Vector2){
            (float)GetRandomValue(0, MAZE_WIDTH * TILE_SIZE),
            (float)GetRandomValue(0, MAZE_HEIGHT * TILE_SIZE)
    };
}