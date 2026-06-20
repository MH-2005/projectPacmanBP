#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "raylib.h"
#include "points.h"
#include "ghosts.h"
#include "GetNamesAndScores.h"
#include "level.h"

#define MAZE_WIDTH 28
#define MAZE_HEIGHT 30
#define TILE_SIZE 20
#define MAX_PLAYERS 10
#define NAME_SIZE 50

typedef struct {
    char name[NAME_SIZE];
    int score;
    int stars;
    int total_time;
    char time[50];
} Player;

// function prototypes
void resetMaze(void);
void reset_elapsed_time(void);
int comparePlayers(const void *a, const void *b);
int map(void);
void draw_items(Texture2D textures[5], int *ii, int *jj);
void pacman(Texture2D PacMan[16], int i, int j, int currentDirection, int h);
void movement(int *i, int *j, int *currentDirection, int *star,
              int *timer_p, int *timer_q, int *lives, int *wait_f, int *wait_g);
void toLowerCase(char *str);
int GameOver(Texture2D deathAnim[11], Texture2D gameover, Color myColor,
             int i, int j, int star, char name[30],
             Font pacmanFont, int minutes, int seconds);
int save_player(char *player_name, int star, int minutes, int seconds);
void unloadGameResources(Texture2D textures[5], Texture2D PacMan[16], Texture2D ghost[40],
                         Texture2D GHOST_BLUE[4], Texture2D deathAnim[11],
                         Texture2D gameover, Texture2D Heart,
                         Font pacmanFont, Font customFont, Music backgroundMusic);
Vector2 getPlayerStartPosition(void);
int maingame(void);

// ---- Initial maze layout ----
char initialmaze_I[MAZE_HEIGHT][MAZE_WIDTH] = {
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
        "#    #        @       #    #",
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

char initialmaze[MAZE_HEIGHT][MAZE_WIDTH];

const int tileSize = 20;

// item counters
static int s_count = 0, g_count = 0, f_count = 0, a_count = 0, m_count = 0;
static int ch_a = 0;   // number of apples currently on the board

// blinking effect for Pac‑Man when gaining a life
static int prev_lives = 3;
static int blink_tr = 255;
static float blink_timer = 0.0f;
static int blink_dir = -1;
static float blink_reset_timer = 0.0f;
static bool blinking = false;

// Pac‑Man animation
static float pac_anim_timer = 0.0f;
static int pac_current_frame = 0;
static int pac_frame_dir = 1;

// movement & power‑up timers
static float move_timer = 0.0f;
static float move_frameTime = 0.2f;
static float speed_timer_f = 0.0f;   // speed boost (pepper)
static float speed_timer_g = 0.0f;   // ghost power (cherry)
static int ff = 0;   // active pepper count
static int gg = 0;   // active cherry count
static bool movement_started = false;

// death animation
static bool death_anim_done = false;
static int death_choice = 0;
static int death_current_frame = 0;
static float death_timer = 0.0f;

// ---- comparison for high‑score table ----
int comparePlayers(const void *a, const void *b) {
    Player *playerA = (Player *) a;
    Player *playerB = (Player *) b;
    if (playerB->stars != playerA->stars) {
        return playerB->stars - playerA->stars;
    }
    return playerA->total_time - playerB->total_time;
}

// ---- helper structures & functions for item placement ----
typedef struct {
    int i;
    int j;
} CellPos;

static CellPos empty_cells[MAZE_HEIGHT * MAZE_WIDTH];
static int empty_count = 0;

static void build_empty_cells_list(void) {
    empty_count = 0;
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (initialmaze[i][j] == ' ') {
                empty_cells[empty_count].i = i;
                empty_cells[empty_count].j = j;
                empty_count++;
            }
        }
    }
}

static CellPos pick_random_empty(void) {
    if (empty_count <= 0) {
        CellPos fallback = {14, 14};
        return fallback;
    }
    int idx = rand() % empty_count;
    return empty_cells[idx];
}

// ---- immediately respawn a collected item at a safe distance from the player ----
void respawn_item(char itemType, int player_i, int player_j) {
#define RESPAWN_MIN_DIST 10.0f
    CellPos candidates[MAZE_HEIGHT * MAZE_WIDTH];
    int cand_count = 0;

    // first, look for empty cells far enough from the player
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (initialmaze[i][j] == ' ') {
                if (get_distance(i, j, player_i, player_j) >= RESPAWN_MIN_DIST) {
                    candidates[cand_count].i = i;
                    candidates[cand_count].j = j;
                    cand_count++;
                }
            }
        }
    }

    // fallback: if no distant cell exists, use any empty cell
    if (cand_count == 0) {
        for (int i = 0; i < MAZE_HEIGHT; i++) {
            for (int j = 0; j < MAZE_WIDTH; j++) {
                if (initialmaze[i][j] == ' ') {
                    candidates[cand_count].i = i;
                    candidates[cand_count].j = j;
                    cand_count++;
                }
            }
        }
    }

    if (cand_count > 0) {
        int idx = rand() % cand_count;
        initialmaze[candidates[idx].i][candidates[idx].j] = itemType;

        // update the item counter
        switch (itemType) {
            case 's': s_count++; break;
            case 'g': g_count++; break;
            case 'f': f_count++; break;
            case 'a': a_count++; break;
            case 'm': m_count++; break;
        }
    }
}

// ---- initial placement of items at game start ----
static void place_items(void) {
    build_empty_cells_list();

    for (int attempt = 0; attempt < 1000 &&
                          (s_count < 10 || g_count < 2 || f_count < 2 || a_count < 1 || m_count < 2);
         attempt++) {
        CellPos cell = pick_random_empty();
        int ci = cell.i;
        int cj = cell.j;

        if (s_count < 10) {
            if (initialmaze[ci][cj] == ' ') { initialmaze[ci][cj] = 's'; s_count++; }
        } else if (g_count < 2) {
            if (initialmaze[ci][cj] == ' ') { initialmaze[ci][cj] = 'g'; g_count++; }
        } else if (f_count < 2) {
            if (initialmaze[ci][cj] == ' ') { initialmaze[ci][cj] = 'f'; f_count++; }
        } else if (a_count < 1) {
            if (initialmaze[ci][cj] == ' ') { initialmaze[ci][cj] = 'a'; a_count++; }
        } else if (m_count < 2) {
            if (initialmaze[ci][cj] == ' ') { initialmaze[ci][cj] = 'm'; m_count++; }
        }
    }
}

// ---- draw the maze walls ----
int map(void) {
    Color MY_BLUE = (Color){50, 120, 140, 255};
    const int borderThickness = 4;
    int count = 0;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * tileSize + 70;
            int y = i * tileSize + 70;

            if (initialmaze[i][j] == '#') {
                DrawRectangle(x, y, tileSize, tileSize, MY_BLUE);
            } else if (initialmaze[i][j] == '_') {
                DrawRectangle(x, y, tileSize, tileSize, (Color){20, 15, 85, 255});
                DrawRectangle(x + borderThickness, y + borderThickness,
                              tileSize - 2 * borderThickness,
                              tileSize - 2 * borderThickness, (Color){50, 120, 140, 150});
            } else if (initialmaze[i][j] == ' ') {
                count++;
            }
        }
    }
    return count;
}

// ---- draw items and locate Pac‑Man start ----
void draw_items(Texture2D textures[5], int *ii, int *jj) {
    ch_a = 0;
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * tileSize + 70;
            int y = i * tileSize + 70;
            char cell = initialmaze[i][j];
            if (cell == 's') {
                DrawTexture(textures[0], x, y, WHITE);
            } else if (cell == 'g') {
                DrawTexture(textures[1], x, y, WHITE);
            } else if (cell == 'f') {
                DrawTexture(textures[2], x, y, WHITE);
            } else if (cell == 'a') {
                DrawTexture(textures[3], x, y, WHITE);
                ch_a++;
            } else if (cell == 'm') {
                DrawTexture(textures[4], x, y, WHITE);
            } else if (cell == '@') {
                *ii = i;
                *jj = j;
            }
        }
    }
}

// ---- draw Pac‑Man with animation and blinking ----
void pacman(Texture2D PacMan[16], int i, int j, int currentDirection, int h) {
    (void) h;

    pac_anim_timer += GetFrameTime();
    if (pac_anim_timer >= 0.1f) {
        pac_anim_timer = 0.0f;
        pac_current_frame += pac_frame_dir;
        if (pac_current_frame >= 3) { pac_current_frame = 3; pac_frame_dir = -1; }
        else if (pac_current_frame <= 0) { pac_current_frame = 0; pac_frame_dir = 1; }
    }

    int finalFrame = currentDirection * 4 + pac_current_frame;
    if (finalFrame < 0 || finalFrame >= 16) finalFrame = currentDirection * 4;

    int x = j * tileSize + 70;
    int y = i * tileSize + 70;

    // blinking when invulnerable after gaining a life
    if (blinking) {
        blink_timer += GetFrameTime();
        if (blink_timer >= 0.1f) {
            blink_timer = 0.0f;
            blink_tr += blink_dir * 50;
            if (blink_tr <= 100) { blink_tr = 100; blink_dir = 1; }
            if (blink_tr >= 255) { blink_tr = 255; blink_dir = -1; }
        }
        blink_reset_timer += GetFrameTime();
        if (blink_reset_timer >= 1.0f) {
            blink_reset_timer = 0.0f;
            blink_tr = 255;
            blinking = false;
        }
    }

    Color semiTransparentWhite = (Color){255, 255, 255, blink_tr};
    DrawTextureEx(PacMan[finalFrame], (Vector2){x, y}, 0.0f, 1, semiTransparentWhite);
}

// ---- check if a cell is walkable by Pac‑Man ----
static bool is_walkable(int i, int j) {
    if (i < 0 || i >= MAZE_HEIGHT || j < 0 || j >= MAZE_WIDTH) return false;
    char c = initialmaze[i][j];
    return c != '#' && c != '_';
}

// ---- handle Pac‑Man movement, item collection, and power‑up timers ----
void movement(int *i, int *j, int *currentDirection, int *star,
              int *timer_p, int *timer_q, int *lives, int *wait_f, int *wait_g) {
    // start movement on first key press
    if (!movement_started && (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
                              IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))) {
        movement_started = true;
    }
    if (!movement_started) return;

    // change direction (only if the adjacent cell is walkable)
    if (IsKeyDown(KEY_RIGHT) && is_walkable(*i, *j + 1)) *currentDirection = 0;
    else if (IsKeyDown(KEY_LEFT) && is_walkable(*i, *j - 1)) *currentDirection = 1;
    else if (IsKeyDown(KEY_UP) && is_walkable(*i - 1, *j)) *currentDirection = 2;
    else if (IsKeyDown(KEY_DOWN) && is_walkable(*i + 1, *j)) *currentDirection = 3;

    move_timer += GetFrameTime();
    if (move_timer >= move_frameTime) {
        move_timer = 0.0f;
        int new_i = *i, new_j = *j;

        if (*currentDirection == 0 && is_walkable(*i, *j + 1)) new_j++;
        else if (*currentDirection == 1 && is_walkable(*i, *j - 1)) new_j--;
        else if (*currentDirection == 2 && is_walkable(*i - 1, *j)) new_i--;
        else if (*currentDirection == 3 && is_walkable(*i + 1, *j)) new_i++;

        if (new_i != *i || new_j != *j) {
            char dest = initialmaze[new_i][new_j];
            char eaten_item = 0;   // remember which item was collected

            // collect item effects
            if (dest == 's') {
                (*star)++;
                s_count--;
                eaten_item = 's';
            } else if (dest == 'f') {
                move_frameTime = 0.1f;      // speed boost
                *wait_f += 10;
                ff++;
                speed_timer_f = 0;
                f_count--;
                eaten_item = 'f';
            } else if (dest == 'a') {
                (*lives)++;
                a_count--;
                eaten_item = 'a';
            } else if (dest == 'm') {
                (*lives)--;
                m_count--;
                eaten_item = 'm';
            } else if (dest == 'g') {
                *wait_g += 10;
                gg++;
                speed_timer_g = 0;
                g_count--;
                eaten_item = 'g';
            }

            // move Pac‑Man
            initialmaze[*i][*j] = ' ';
            initialmaze[new_i][new_j] = '@';
            *i = new_i;
            *j = new_j;

            // immediately respawn the collected item
            if (eaten_item != 0) {
                respawn_item(eaten_item, *i, *j);
            }
        }
    }

    // speed boost (pepper) timer
    speed_timer_f += GetFrameTime();
    if (ff > 0 && *wait_f > 0 && speed_timer_f >= (float)*wait_f) {
        move_frameTime = 0.2f;
        speed_timer_f = 0.0f;
        *wait_f = 0;
        f_count -= ff;
        if (f_count < 0) f_count = 0;
        ff = 0;
    }

    // ghost power (cherry) timer
    speed_timer_g += GetFrameTime();
    if (gg > 0 && *wait_g > 0 && speed_timer_g >= (float)*wait_g) {
        speed_timer_g = 0.0f;
        *wait_g = 0;
        gg = 0;
    }

    // remaining time on power‑ups (for display)
    *timer_p = *wait_f > 0 ? (*wait_f - (int)speed_timer_f) : 0;
    *timer_q = *wait_g > 0 ? (*wait_g - (int)speed_timer_g) : 0;
}

// ---- utility to convert a string to lowercase ----
void toLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}

// ---- game over screen ----
int GameOver(Texture2D deathAnim[11], Texture2D gameover, Color myColor,
             int i, int j, int star, char name[30],
             Font pacmanFont, int minutes, int seconds) {
    BeginDrawing();
    ClearBackground(myColor);

    if (!death_anim_done) {
        death_timer += GetFrameTime();
        if (death_timer >= 0.2f) {
            death_timer = 0.0f;
            if (death_current_frame < 10) death_current_frame++;
        }

        int x = j * tileSize + 70;
        int y = i * tileSize + 70;
        DrawTexture(deathAnim[death_current_frame], x, y, WHITE);
        if (death_current_frame >= 10) death_anim_done = true;
    } else {
        Color tc[2] = {RAYWHITE, RAYWHITE};
        tc[death_choice] = GREEN;
        DrawTexture(gameover, 0, 0, WHITE);
        DrawText("Give it another shot!", 120, 630, 40, tc[0]);
        DrawText("Back to Main Menu", 140, 700, 40, tc[1]);

        if (IsKeyPressed(KEY_DOWN)) death_choice = (death_choice + 1) % 2;
        if (IsKeyPressed(KEY_UP))   death_choice = (death_choice - 1 + 2) % 2;
        if (IsKeyPressed(KEY_ENTER)) {
            int choice = death_choice;
            death_anim_done = false;
            death_current_frame = 0;
            death_timer = 0.0f;
            death_choice = 0;
            EndDrawing();
            return choice;
        }

        toLowerCase(name);
        DrawTextEx(pacmanFont, name, (Vector2){230, 530}, 20, 2, RAYWHITE);
        DrawTextEx(pacmanFont, "score : ", (Vector2){500, 530}, 20, 2, RAYWHITE);
        char sc[20];
        snprintf(sc, sizeof(sc), "%d", star * 10);
        DrawText(sc, 630, 525, 30, RAYWHITE);
        char t[100];
        snprintf(t, sizeof(t), "You have been playing for [%02d:%02d]. Keep it up!", minutes, seconds);
        DrawText(t, 355, 580, 15, RAYWHITE);
    }

    EndDrawing();
    return -1;
}

// ---- save player record ----
int save_player(char *player_name, int star, int minutes, int seconds) {
    Player players[MAX_PLAYERS + 1];
    int player_count = 0;

    FILE *fp = fopen("saves.bin", "rb");
    if (fp != NULL) {
        while (fread(&players[player_count], sizeof(Player), 1, fp) == 1) {
            player_count++;
            if (player_count >= MAX_PLAYERS) break;
        }
        fclose(fp);
    }

    Player new_player;
    memset(&new_player, 0, sizeof(Player));
    strncpy(new_player.name, player_name, NAME_SIZE - 1);
    new_player.stars = star;
    new_player.total_time = minutes * 60 + seconds;

    time_t current_time;
    struct tm *local_time;
    time(&current_time);
    local_time = localtime(&current_time);
    strftime(new_player.time, sizeof(new_player.time), "%Y/%m/%d", local_time);

    players[player_count] = new_player;
    player_count++;

    qsort(players, player_count, sizeof(Player), comparePlayers);

    if (player_count > MAX_PLAYERS) player_count = MAX_PLAYERS;

    fp = fopen("saves.bin", "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    fwrite(players, sizeof(Player), player_count, fp);
    fclose(fp);
    return EXIT_SUCCESS;
}

// ---- main game function ----
int maingame(void) {
    Music backgroundMusic = LoadMusicStream("../assets/audio/take_on_me_small.mp3");
    PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.5f);

    int r = -1;
    Font customFont = LoadFontEx("../assets/fonts/calibri.ttf", 32, NULL, 0);
    char player_name[30];
    getnames(player_name, 30, customFont);
    Font pacmanFont = LoadFont("../assets/fonts/PAC-FONT.TTF");

    resetMaze();
    reset_elapsed_time();
    reset_ghost_search();

    // reset animation & power‑up states
    prev_lives = 3;
    blink_tr = 255;
    blink_timer = 0.0f; blink_dir = -1; blink_reset_timer = 0.0f; blinking = false;
    pac_anim_timer = 0.0f; pac_current_frame = 0; pac_frame_dir = 1;
    move_timer = 0.0f; move_frameTime = 0.2f;
    speed_timer_f = 0.0f; speed_timer_g = 0.0f;
    ff = 0; gg = 0;
    movement_started = false;
    death_anim_done = false; death_choice = 0; death_current_frame = 0; death_timer = 0.0f;

    // load textures
    Texture2D textures[5];
    textures[0] = LoadTexture("../assets/sprites/Points/star.png");
    textures[1] = LoadTexture("../assets/sprites/Points/cherry.png");
    textures[2] = LoadTexture("../assets/sprites/Points/pepper.png");
    textures[3] = LoadTexture("../assets/sprites/Points/apple.png");
    textures[4] = LoadTexture("../assets/sprites/Points/mushroom.png");

    Texture2D PacMan[16];
    char *Direction[] = {"Right", "Left", "Up", "Down"};
    for (int n = 0; n < 4; n++) {
        for (int m = 0; m < 4; m++) {
            char location[128];
            snprintf(location, sizeof(location),
                     "../assets/sprites/pac/Direction/%s/%d.png", Direction[n], m + 1);
            PacMan[n * 4 + m] = LoadTexture(location);
        }
    }

    Texture2D ghost[40];
    char ghost_location[5][10] = {"blue", "green", "orange", "pink", "red"};
    char ghost_dir[8][3] = {"D1","D2","L1","L2","R1","R2","U1","U2"};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            char location[124];
            snprintf(location, sizeof(location),
                     "../assets/sprites/ghosts/%s/%s.png", ghost_location[i], ghost_dir[j]);
            ghost[i * 8 + j] = LoadTexture(location);
        }
    }

    Texture2D GHOST_BLUE[4];
    char *g_b[4] = {"blue1", "blue2", "white1", "white2"};
    for (int n = 0; n < 4; n++) {
        char location[100];
        snprintf(location, sizeof(location),
                 "../assets/sprites/ghosts/vulnerable/%s.png", g_b[n]);
        GHOST_BLUE[n] = LoadTexture(location);
    }

    Texture2D deathAnim[11];
    for (int i = 0; i < 11; i++) {
        char location[150];
        snprintf(location, sizeof(location),
                 "../assets/sprites/pac/deathAnim/death%d.png", i + 1);
        deathAnim[i] = LoadTexture(location);
    }

    Texture2D gameover = LoadTexture("../assets/sprites/GameOver.png");
    Texture2D Heart = LoadTexture("../assets/sprites/heart.png");

    RandomGhostPosition();
    place_items();

    Color myColor = (Color){20, 25, 30, 255};
    SetTargetFPS(60);

    int ii = 0, jj = 0;
    int currentDirection = 0;
    int star = 0, timer_p = 0, timer_q = 0;
    int minutes = 0, seconds = 0;
    int lives = 3;
    int wait_f = 0, wait_g = 0;

    // ---------- MAIN GAME LOOP ----------
    while (!WindowShouldClose()) {
        UpdateMusicStream(backgroundMusic);

        if (lives > 0) {
            // blinking invulnerability when gaining a life
            if (lives > prev_lives) {
                blinking = true;
                blink_timer = 0.0f;
                blink_reset_timer = 0.0f;
                blink_tr = 255;
            }
            prev_lives = lives;

            BeginDrawing();
            ClearBackground(myColor);

            map();
            draw_items(textures, &ii, &jj);

            // move Pac‑Man and handle item collection
            movement(&ii, &jj, &currentDirection, &star,
                     &timer_p, &timer_q, &lives, &wait_f, &wait_g);

            pacman(PacMan, ii, jj, currentDirection, lives);
            draw_score(star, timer_p, timer_q);
            heart(lives, Heart);
            print_Name_Time(player_name, customFont, &minutes, &seconds);

            // UNIFIED ghost update & draw (handles both normal and vulnerable mode)
            update_and_draw_ghosts(ghost, GHOST_BLUE, ii, jj, &lives, &star, timer_q);

            EndDrawing();
        } else {
            int ch = GameOver(deathAnim, gameover, myColor, ii, jj, star,
                              player_name, pacmanFont, minutes, seconds);
            if (ch == 0) { r = 0; break; }
            if (ch == 1) { r = 1; break; }
        }
    }

    save_player(player_name, star * 10, minutes, seconds);
    unloadGameResources(textures, PacMan, ghost, GHOST_BLUE, deathAnim, gameover, Heart,
                        pacmanFont, customFont, backgroundMusic);
    return r;
}

// ---- reset the maze to its initial state ----
void resetMaze(void) {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            initialmaze[i][j] = initialmaze_I[i][j];
        }
    }
    s_count = 0; g_count = 0; f_count = 0; a_count = 0; m_count = 0;
    ch_a = 0;
}

// ---- unload all textures, fonts and music ----
void unloadGameResources(Texture2D textures[5], Texture2D PacMan[16], Texture2D ghost[40],
                         Texture2D GHOST_BLUE[4], Texture2D deathAnim[11],
                         Texture2D gameover, Texture2D Heart,
                         Font pacmanFont, Font customFont, Music backgroundMusic) {
    for (int i = 0; i < 5; i++) UnloadTexture(textures[i]);
    for (int i = 0; i < 16; i++) UnloadTexture(PacMan[i]);
    for (int i = 0; i < 40; i++) UnloadTexture(ghost[i]);
    for (int i = 0; i < 4; i++) UnloadTexture(GHOST_BLUE[i]);
    for (int i = 0; i < 11; i++) UnloadTexture(deathAnim[i]);
    UnloadTexture(gameover);
    UnloadTexture(Heart);
    UnloadFont(pacmanFont);
    UnloadFont(customFont);
    StopMusicStream(backgroundMusic);
    UnloadMusicStream(backgroundMusic);
}

// ---- find the cell containing '@' ----
Vector2 getPlayerStartPosition(void) {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (initialmaze[i][j] == '@') {
                return (Vector2){j * TILE_SIZE, i * TILE_SIZE};
            }
        }
    }
    return (Vector2){14 * TILE_SIZE, 23 * TILE_SIZE};
}