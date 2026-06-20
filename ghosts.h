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

#define MAZE_WIDTH           28
#define MAZE_HEIGHT          30
#define GHOST_COUNT          5
#define MIN_GHOST_DISTANCE   1
#define MAX_PATH_LEN         900
#define RESPAWN_DURATION     1.5f      // time a ghost stays invisible after being eaten

typedef struct {
    char color[10];
    char direction;
    int  x;
} pos;

// forward declaration
Vector2 getGhostSpawnPosition(int ghostIndex);

// ---- Maze used for ghost pathfinding (contains only '#', ' ', '_', 'R') ----
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

// Colour / direction -> texture offset lookup
const pos z[20] = {
        {"blue",   'D', 0}, {"blue",   'L', 1}, {"blue",   'R', 2}, {"blue",   'U', 3},
        {"green",  'D', 4}, {"green",  'L', 5}, {"green",  'R', 6}, {"green",  'U', 7},
        {"orange", 'D', 8}, {"orange", 'L', 9}, {"orange", 'R',10}, {"orange", 'U',11},
        {"pink",   'D',12}, {"pink",   'L',13}, {"pink",   'R',14}, {"pink",   'U',15},
        {"red",    'D',16}, {"red",    'L',17}, {"red",    'R',18}, {"red",    'U',19}
};

// ---- Per‑ghost state ----
int  i_pos[GHOST_COUNT], j_pos[GHOST_COUNT];
int  last_direction[GHOST_COUNT] = {0};

// normal animation
static float ghost_anim_timer = 0.0f;
static int   ghost_anim_frame = 0;

// collision cooldown (so ghost doesn't drain lives every frame)
static float last_hit_time = 0.0f;

static bool  global_seeded = false;
static bool  ghost_started = false;   // ghosts only move after player first presses a key

// per‑ghost movement timers (replace old global ghost_move_timer)
static float ghost_move_timers[GHOST_COUNT] = {0.0f};

// BFS path storage
static int  path_index[GHOST_COUNT] = {0};
static char path_store[GHOST_COUNT][MAX_PATH_LEN];
static bool path_found[GHOST_COUNT] = {false};

// vulnerable‑ghost animation / respawn
static float se_timer = 0.0f;
static int   se_frames = 0;

static float respawn_timer[GHOST_COUNT];
static bool  is_respawning[GHOST_COUNT] = {false};

// ---- helper: seed RNG once ----
static void seed_rng_once(void) {
    if (!global_seeded) {
        srand((unsigned int) time(NULL));
        global_seeded = true;
    }
}

// ---- distance between two cells ----
float get_distance(int x1, int y1, int x2, int y2) {
    int dx = x1 - x2;
    int dy = y1 - y2;
    return (float) sqrt(dx * dx + dy * dy);
}

// ---- check whether a cell is safe for a specific ghost (no other ghosts too close) ----
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

// ---- draw a normal ghost (used inside the unified function) ----
static void draw_normal_ghost(pos g, Texture2D ghost_tex[40], int ii, int jj,
                              int pac_i, int pac_j, int *lives) {
    float current_time = GetTime();
    for (int i = 0; i < 20; i++) {
        if (!strcmp(g.color, z[i].color) && g.direction == z[i].direction) {
            // handle collision with Pac‑Man (normal mode -> lose a life, max 1 per second)
            if (ii == pac_i && jj == pac_j) {
                if (current_time - last_hit_time >= 1.0f) {
                    (*lives)--;
                    last_hit_time = current_time;
                }
            }
            DrawTexture(ghost_tex[z[i].x * 2 + ghost_anim_frame],
                        jj * TILE_SIZE + 70, ii * TILE_SIZE + 70, WHITE);
            break;
        }
    }
}

// ---- find a random empty cell at least MIN_GHOST_DISTANCE from the player ----
void random_place_target(int ghost_index, int *target_row, int *target_col,
                         int player_i, int player_j) {
    int available_positions[MAZE_HEIGHT * MAZE_WIDTH][2];
    int count = 0;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (initialmaze_G[i][j] == ' ' &&
                get_distance(i, j, player_i, player_j) >= MIN_GHOST_DISTANCE &&
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

    int r = rand() % count;
    *target_row = available_positions[r][0];
    *target_col = available_positions[r][1];
}

// ---- BFS queue helpers ----
typedef struct {
    int i, j;
    char *path;
} QueueElement;

static void bfs_free_remaining(QueueElement *queue, int front, int rear) {
    for (int k = front; k < rear; k++) {
        free(queue[k].path);
        queue[k].path = NULL;
    }
}

// ---- BFS pathfinding (returns a path string of 'D','U','R','L') ----
void FindPathUsingBFS(int ghost_index, int target_row, int target_col,
                      int now_row, int now_col, char *path) {
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

        for (int d = 0; d < 4; d++) {
            int ni = ci + dr[d];
            int nj = cj + dc[d];
            if (ni >= 0 && ni < MAZE_HEIGHT && nj >= 0 && nj < MAZE_WIDTH &&
                initialmaze_G[ni][nj] != '#' && !visited[ni][nj] &&
                is_position_safe(ghost_index, ni, nj)) {
                int clen = (int) strlen(cpath);
                char *new_path = malloc(clen + 2);
                strcpy(new_path, cpath);
                new_path[clen] = dir[d];
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

    // no path found
    path[0] = '\0';
    bfs_free_remaining(queue, front, rear);
    free(queue);
}

// ---- place all ghosts randomly inside the 'R' areas at start ----
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
        is_respawning[i] = false;
        respawn_timer[i] = 0.0f;
        ghost_move_timers[i] = 0.0f;
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
            int r = rand() % count;
            i_pos[m] = available_positions[r][0];
            j_pos[m] = available_positions[r][1];
            initialmaze_G[i_pos[m]][j_pos[m]] = 'r';
        }
    }
}

// ---- convert direction character to index (0‑3) ----
int get_direction_index(char direction) {
    switch (direction) {
        case 'D': return 0;
        case 'U': return 1;
        case 'R': return 2;
        case 'L': return 3;
        default:  return 0;
    }
}

// ---- move one ghost along its BFS path; now uses a per‑ghost timer ----
void movementGhost(int ghost_index, int *direction_index, char path_i[MAX_PATH_LEN],
                   bool *found, float *move_timer) {
    if (!ghost_started) return;

    *move_timer += GetFrameTime();
    if (*move_timer < 0.2f) return;   // movement speed: 5 tiles per second
    *move_timer = 0.0f;

    // if path exhausted, restart search
    if (!path_i[path_index[ghost_index]]) {
        path_index[ghost_index] = 0;
        *found = false;
    }

    char dir_char = path_i[path_index[ghost_index]];
    *direction_index = get_direction_index(dir_char);
    last_direction[ghost_index] = *direction_index;

    int ni = i_pos[ghost_index];
    int nj = j_pos[ghost_index];
    bool can_move = false;

    switch (dir_char) {
        case 'R':
            if (nj + 1 < MAZE_WIDTH && initialmaze_G[ni][nj + 1] != '#') { nj++; can_move = true; }
            break;
        case 'L':
            if (nj - 1 >= 0 && initialmaze_G[ni][nj - 1] != '#') { nj--; can_move = true; }
            break;
        case 'U':
            if (ni - 1 >= 0 && initialmaze_G[ni - 1][nj] != '#') { ni--; can_move = true; }
            break;
        case 'D':
            if (ni + 1 < MAZE_HEIGHT && initialmaze_G[ni + 1][nj] != '#') { ni++; can_move = true; }
            break;
    }

    if (can_move && is_position_safe(ghost_index, ni, nj)) {
        i_pos[ghost_index] = ni;
        j_pos[ghost_index] = nj;
        path_index[ghost_index]++;
    } else {
        // invalid move – force a new path next frame
        path_index[ghost_index] = 0;
        *found = false;
    }
}

// =====================================================================
//  UNIFIED GHOST UPDATE & DRAW
//  This replaces insert_ghost() and eat_ghost().
//  Handles both normal and vulnerable mode, respawning, collisions,
//  and drawing in one place.
// =====================================================================
void update_and_draw_ghosts(Texture2D ghost_tex[40], Texture2D ghost_vuln_tex[4],
                            int pac_i, int pac_j, int *lives, int *star, int wait_g) {
    // ---------- 1. Start ghosts on first player key press ----------
    if (!ghost_started) {
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
            IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT)) {
            ghost_started = true;
        }
    }

    // ---------- 2. Animate normal ghost sprites ----------
    ghost_anim_timer += GetFrameTime();
    if (ghost_anim_timer >= 0.8f) {
        ghost_anim_timer = 0.0f;
        ghost_anim_frame = !ghost_anim_frame;   // toggle 0/1
    }

    // ---------- 3. Vulnerable ghost animation ----------
    if (wait_g > 0) {
        se_timer += GetFrameTime();
        if (se_timer >= 0.8f) {
            se_timer = 0.0f;
            int max_frames = (wait_g >= 2) ? 2 : 4;   // blink faster when about to end
            se_frames = (se_frames + 1) % max_frames;
        }
    }

    // ---------- 4. Process each ghost ----------
    static const char *colors[GHOST_COUNT] = {"blue", "green", "orange", "pink", "red"};
    static const char directions[4] = {'D', 'U', 'R', 'L'};

    for (int i = 0; i < GHOST_COUNT; i++) {
        // ----- 4a. If this ghost is dead / respawning -----
        if (is_respawning[i]) {
            respawn_timer[i] += GetFrameTime();
            if (respawn_timer[i] >= RESPAWN_DURATION) {
                // respawn at a random location far from Pac‑Man
                int tr, tc;
                random_place_target(i, &tr, &tc, pac_i, pac_j);
                if (tr != -1) {
                    i_pos[i] = tr;
                    j_pos[i] = tc;
                } else {
                    // fallback: use the ghost house centre
                    i_pos[i] = 14;
                    j_pos[i] = 14;
                }
                is_respawning[i] = false;
                respawn_timer[i] = 0.0f;
                path_found[i] = false;    // force new BFS path
                path_index[i] = 0;
                path_store[i][0] = '\0';
                ghost_move_timers[i] = 0.0f;
            }
            // do NOT draw, do NOT move – ghost is invisible
            continue;
        }

        // ----- 4b. Compute a BFS path if needed (always, regardless of mode) -----
        if (!path_found[i]) {
            int tr, tc;
            if (wait_g > 0) {
                // vulnerable mode: ghosts flee from Pac‑Man → choose random target
                random_place_target(i, &tr, &tc, pac_i, pac_j);
            } else {
                // normal mode: two random ghosts chase Pac‑Man, the rest wander
                int chaser1 = rand() % GHOST_COUNT;
                int chaser2 = rand() % GHOST_COUNT;
                while (chaser2 == chaser1) chaser2 = rand() % GHOST_COUNT;
                if (i == chaser1 || i == chaser2) {
                    tr = pac_i;
                    tc = pac_j;
                } else {
                    random_place_target(i, &tr, &tc, pac_i, pac_j);
                }
            }
            if (tr != -1 && tc != -1) {
                FindPathUsingBFS(i, tr, tc, i_pos[i], j_pos[i], path_store[i]);
                if (path_store[i][0] != '\0') {
                    path_found[i] = true;
                }
            }
        }

        // ----- 4c. Move the ghost (if movement has started) -----
        int dir_idx = last_direction[i];
        movementGhost(i, &dir_idx, path_store[i], &path_found[i], &ghost_move_timers[i]);

        // ----- 4d. Collision detection with Pac‑Man -----
        bool collides = (i_pos[i] == pac_i && j_pos[i] == pac_j);

        if (collides) {
            if (wait_g > 0) {
                // ---------- vulnerable mode: eat the ghost ----------
                *star += 2;
                // start respawn (ghost becomes invisible)
                is_respawning[i] = true;
                respawn_timer[i] = 0.0f;
                path_found[i] = false;   // discard old path
                path_index[i] = 0;
                path_store[i][0] = '\0';
                ghost_move_timers[i] = 0.0f;
                // skip drawing this ghost for now (will be invisible)
                continue;
            } else {
                // ---------- normal mode: lose a life ----------
                float now = GetTime();
                if (now - last_hit_time >= 1.0f) {
                    (*lives)--;
                    last_hit_time = now;
                }
            }
        }

        // ----- 4e. Draw the ghost (if not respawning) -----
        if (wait_g > 0) {
            // vulnerable mode: draw the special blue/white texture
            DrawTexture(ghost_vuln_tex[se_frames],
                        j_pos[i] * TILE_SIZE + 70, i_pos[i] * TILE_SIZE + 70, WHITE);
        } else {
            // normal mode: draw with colour & direction
            pos g;
            strcpy(g.color, colors[i]);
            g.direction = directions[last_direction[i]];  // 0‑3 -> 'D','U','R','L'
            // draw (collision damage is handled above, no need to check again)
            for (int k = 0; k < 20; k++) {
                if (!strcmp(g.color, z[k].color) && g.direction == z[k].direction) {
                    DrawTexture(ghost_tex[z[k].x * 2 + ghost_anim_frame],
                                j_pos[i] * TILE_SIZE + 70, i_pos[i] * TILE_SIZE + 70, WHITE);
                    break;
                }
            }
        }
    }
}

// ---- reset all ghost state (called when restarting game) ----
void reset_ghost_search(void) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        last_direction[i] = 0;
        path_found[i] = false;
        path_index[i] = 0;
        path_store[i][0] = '\0';
        is_respawning[i] = false;
        respawn_timer[i] = 0.0f;
        ghost_move_timers[i] = 0.0f;
    }
    ghost_started = false;
    ghost_anim_timer = 0.0f;
    ghost_anim_frame = 0;
    last_hit_time = 0.0f;
    se_timer = 0.0f;
    se_frames = 0;
}

// ---- helper to get spawn position (not used in the new logic, kept for compatibility) ----
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