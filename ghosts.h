#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "raylib.h"
#include "raymath.h"

#ifndef TILE_SIZE
#define TILE_SIZE 20
#endif

#define MAZE_WIDTH 28
#define MAZE_HEIGHT 30
#define GHOST_COUNT 5
#define MIN_GHOST_DISTANCE 1
#define MAX_PATH_LEN 900

typedef struct {
    char color[10];
    char direction;
    int x;
} pos;

Vector2 getGhostSpawnPosition(int ghostIndex);

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

static float ghost_anim_timer = 0.0f;
static int ghost_anim_frame = 0;
static float last_hit_time = 0.0f;

static bool global_seeded = false;
static bool ghost_started = false;
static float ghost_move_timer = 0.0f;
static int path_index[GHOST_COUNT] = {0};
static char path_store[GHOST_COUNT][MAX_PATH_LEN];
static bool path_found[GHOST_COUNT] = {false};

static float se_timer = 0.0f;
static float respawn_timer[GHOST_COUNT];
static bool is_respawning[GHOST_COUNT] = {false};
static int se_frames = 0;

static void seed_rng_once(void) {
    if (!global_seeded) {
        srand((unsigned int) time(NULL));
        global_seeded = true;
    }
}

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
    float current_time = GetTime();

    for (int i = 0; i < 20; i++) {
        if (!strcmp(g.color, z[i].color) && g.direction == z[i].direction) {
            if (ii == ii_p && jj == jj_p) {
                if (current_time - last_hit_time >= 1.0f) {
                    (*h)--;
                    last_hit_time = current_time;
                }
            }
            DrawTexture(ghost[z[i].x * 2 + ghost_anim_frame], jj * 20 + 70, ii * 20 + 70, WHITE);
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

static void bfs_free_remaining(QueueElement *queue, int front, int rear) {
    for (int k = front; k < rear; k++) {
        if (queue[k].path != NULL) {
            free(queue[k].path);
            queue[k].path = NULL;
        }
    }
}

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
        int ci = queue[front].i;
        int cj = queue[front].j;
        char *cpath = queue[front].path;
        front++;

        if (ci == target_row && cj == target_col) {
            strncpy(path, cpath, MAX_PATH_LEN - 1);
            path[MAX_PATH_LEN - 1] = '\0';
            free(cpath);
            bfs_free_remaining(queue, front, rear);
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
                int clen = (int) strlen(cpath);
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
        free(cpath);
    }

    path[0] = '\0';
    bfs_free_remaining(queue, front, rear);
    free(queue);
}

void RandomGhostPosition(void) {
    static const char initialmaze_G_I[MAZE_HEIGHT][MAZE_WIDTH] = {
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

    seed_rng_once();

    for (int i = 0; i < GHOST_COUNT; i++) {
        last_direction[i] = 0;
        path_found[i] = false;
        path_index[i] = 0;
        path_store[i][0] = '\0';
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

void movementGhost(int ghost_index, int *j, char path_i[MAX_PATH_LEN], bool *found) {
    if (!ghost_started) return;

    ghost_move_timer += GetFrameTime();
    if (ghost_move_timer < 0.2f) return;
    ghost_move_timer = 0.0f;

    if (!path_i[path_index[ghost_index]]) {
        path_index[ghost_index] = 0;
        *found = false;
    }

    char current_direction = path_i[path_index[ghost_index]];
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
        path_index[ghost_index]++;
    } else {
        path_index[ghost_index] = 0;
        *found = false;
    }
}

void insert_ghost(Texture2D ghost[40], int ii, int jj, int *h) {
    const char *Colors[GHOST_COUNT] = {"blue", "green", "orange", "pink", "red"};
    const char Directions[4] = {'D', 'U', 'R', 'L'};

    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
        IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) {
        ghost_started = true;
    }

    ghost_anim_timer += GetFrameTime();
    if (ghost_anim_timer >= 0.8f) {
        ghost_anim_timer = 0.0f;
        ghost_anim_frame = !ghost_anim_frame;
    }

    for (int i = 0; i < GHOST_COUNT; i++) {
        if (!path_found[i]) {
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
                FindPathUsingBFS(i, target_row, target_col, i_pos[i], j_pos[i], path_store[i]);
                if (path_store[i][0] != '\0') {
                    path_found[i] = true;
                }
            }
        }

        int direction_index = last_direction[i];
        pos g = {.direction = Directions[direction_index]};
        strcpy(g.color, Colors[i]);

        movementGhost(i, &direction_index, path_store[i], &path_found[i]);
        ghost_show(g, ghost, i_pos[i], j_pos[i], ii, jj, h);
    }
}

void GHOST_S_E(Texture2D ghost[4], int index, int *ii, int *jj, int ii_p, int jj_p, int *star, const int *wait_g) {
    float current_time = GetTime();
    se_timer += GetFrameTime();

    int c_w = (*wait_g >= 2) ? 2 : 4;

    if (se_timer >= 0.8f) {
        se_timer = 0.0f;
        se_frames = (se_frames + 1) % c_w;
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
            if (target_row != -1 && target_col != -1) {
                *ii = target_row;
                *jj = target_col;
            }
            is_respawning[index] = false;
        }
        DrawTexture(ghost[se_frames], *jj * 20 + 70, *ii * 20 + 70, WHITE);
    }
}

void eat_ghost(Texture2D ghost[4], int ii, int jj, int *star, const int *wait_g) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        if (!path_found[i]) {
            int target_row, target_col;
            random_place_target(i, &target_row, &target_col, ii, jj);
            if (target_row != -1 && target_col != -1) {
                FindPathUsingBFS(i, target_row, target_col, i_pos[i], j_pos[i], path_store[i]);
                if (path_store[i][0] != '\0') {
                    path_found[i] = true;
                }
            }
        }
        int direction_index = last_direction[i];
        movementGhost(i, &direction_index, path_store[i], &path_found[i]);
        GHOST_S_E(ghost, i, &i_pos[i], &j_pos[i], ii, jj, star, wait_g);
    }
}

void reset_ghost_search(void) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        last_direction[i] = 0;
        path_found[i] = false;
        path_index[i] = 0;
        path_store[i][0] = '\0';
        is_respawning[i] = false;
        respawn_timer[i] = 0.0f;
    }
    ghost_started = false;
    ghost_move_timer = 0.0f;
    ghost_anim_timer = 0.0f;
    ghost_anim_frame = 0;
    last_hit_time = 0.0f;
    se_timer = 0.0f;
    se_frames = 0;
}

Vector2 getGhostSpawnPosition(int ghostIndex) {
    Vector2 positions[GHOST_COUNT] = {
        {5 * TILE_SIZE, 5 * TILE_SIZE},
        {22 * TILE_SIZE, 5 * TILE_SIZE},
        {5 * TILE_SIZE, 24 * TILE_SIZE},
        {22 * TILE_SIZE, 24 * TILE_SIZE},
        {14 * TILE_SIZE, 14 * TILE_SIZE}
    };
    return positions[ghostIndex % GHOST_COUNT];
}
