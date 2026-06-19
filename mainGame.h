#pragma once   // FIX 1: اضافه شد

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include "raylib.h"
#include "points.h"
#include "ghosts.h"
#include "GetNamesAndScores.h"

#define MAZE_WIDTH 28
#define MAZE_HEIGHT 31
#define TILE_SIZE 20
#define MAX_PLAYERS 10
#define NAME_SIZE 50
#ifndef GHOST_SPEED
#define GHOST_SPEED 2.0f
#endif
#define PACMAN_SPEED 3.0f
#define POWER_PELLET_DURATION 10.0f

typedef enum {
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState;

typedef struct {
    char name[NAME_SIZE];
    int score;
    int stars;
    int total_time;
    char time[50];
    Vector2 position;
    float speed;
    int direction;
    bool isPowered;
    float powerTimer;
} Player;

typedef struct {
    char maze[MAZE_HEIGHT][MAZE_WIDTH];
    Player player;
    GameState state;
    int score;
    float gameTime;
    bool canEatGhosts;
    float ghostTimer;
    int lives;
    GhostManager ghostManager;
} Game;

void initGame(Game *game);
void updateGame(Game *game);
void drawGame(Game *game);
void handleInput(Game *game);
void checkCollisions(Game *game);
void drawMaze(Game *game);
void drawPlayer(Game *game);
void drawScore(Game *game);
void drawTimer(Game *game);
void resetMaze(void);
void RandomGhostPosition(void);
void eat_ghost(Texture2D GHOST_BLUE[4], int ii, int jj, int *star, const int *wait_g);
void insert_ghost(Texture2D ghost[40], int ii, int jj, int *h);
void unloadGameResources(Texture2D textures[5], Texture2D PacMan[16], Texture2D ghost[40],
                         Texture2D GHOST_BLUE[4], Texture2D deathAnim[11],
                         Texture2D gameover, Texture2D Heart,
                         Font pacmanFont, Font customFont, Music backgroundMusic);
void drawGhostsVisual(GhostManager *manager);
Vector2 getPlayerStartPosition(void);
void resetGhostPositions(Game *game);
void checkPowerPellet(Game *game);

char initialmaze[MAZE_HEIGHT][MAZE_WIDTH] = {
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

const int tileSize = 20;
bool can_eat = false;

int comparePlayers(const void *a, const void *b) {
    Player *playerA = (Player *) a;
    Player *playerB = (Player *) b;
    if (playerB->stars != playerA->stars) {
        return playerB->stars - playerA->stars;
    }
    return playerA->total_time - playerB->total_time;
}

int map(void) {
    int count = 0;
    Color MY_BLUE = (Color){50, 120, 140, 255};

    const int borderThickness = 4;
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * tileSize;
            int y = i * tileSize;
            x += 70;
            y += 70;
            if (initialmaze[i][j] == '#') {
                DrawRectangle(x, y, tileSize, tileSize, MY_BLUE);
            } else if (initialmaze[i][j] == '_') {
                DrawRectangle(x, y, tileSize, tileSize, (Color){20, 15, 85, 255});
                DrawRectangle(x + borderThickness, y + borderThickness, tileSize - 2 * borderThickness,
                              tileSize - 2 * borderThickness, (Color){50, 120, 140, 150});
            } else if (initialmaze[i][j] == ' ') {
                count++;
            }
        }
    }
    return count;
}

int s = 0, g = 0, f = 0, a = 0, m = 0, ch_a, C_s = 10;

void insert(int count, Texture2D textures[count], int *ii, int *jj) {
    srand(time(NULL));
    while (true) {
        int random_num = abs(rand()) % count;
        int ch = random_num;
        int i, j;
        for (i = 0; i < MAZE_HEIGHT; i++) {
            for (j = 0; j < MAZE_WIDTH; j++) {
                if (initialmaze[i][j] == ' ') {
                    if (ch >= 0) {
                        ch--;
                    }
                }
                if (ch < 0) {
                    break;
                }
            }
            if (ch < 0) {
                break;
            }
        }
        if (s < 10) {
            if (initialmaze[i][j] != 's') {
                initialmaze[i][j] = 's';
                s++;
            }
        } else if (g < 2) {
            if ((initialmaze[i][j] != 's') &&
                (initialmaze[i][j] != 'g')) {
                initialmaze[i][j] = 'g';
                g++;
            }
        } else if (f < 2) {
            if ((initialmaze[i][j] != 's') &&
                (initialmaze[i][j] != 'g') &&
                (initialmaze[i][j] != 'f')) {
                initialmaze[i][j] = 'f';
                f++;
            }
        } else if (a < 1) {
            if ((initialmaze[i][j] != 's') &&
                (initialmaze[i][j] != 'g') &&
                (initialmaze[i][j] != 'f') &&
                (initialmaze[i][j] != 'a')) {
                initialmaze[i][j] = 'a';
                a++;
            }
        } else if (m < 2) {
            if ((initialmaze[i][j] != 's') &&
                (initialmaze[i][j] != 'g') &&
                (initialmaze[i][j] != 'f') &&
                (initialmaze[i][j] != 'a') &&
                (initialmaze[i][j] != 'm')) {
                initialmaze[i][j] = 'm';
                m++;
            }
        } else {
            break;
        }
    }
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * tileSize;
            int y = i * tileSize;
            x += 70;
            y += 70;
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

int jon = 3;

void pacman(Texture2D PacMan[16], int i, int j, int currentDirection, int h) {
    static int currentFrame = 0;
    static float frameTime = 0.1f;
    static float timer = 0.0f;
    static int direction = 1;

    timer += GetFrameTime();
    if (timer >= frameTime) {
        timer = 0.0f;
        currentFrame += direction;
        if (currentFrame >= 3) {
            currentFrame = 3;
            direction = -1;
        } else if (currentFrame <= 0) {
            currentFrame = 0;
            direction = 1;
        }
    }

    int finalFrame = currentDirection * 4 + currentFrame;
    if (finalFrame < 0 || finalFrame >= 16) {
        finalFrame = currentDirection * 4;
    }

    int x = j * tileSize + 70;
    int y = i * tileSize + 70;

    static int tr = 255;
    if (jon > h) {
        static float timer_b = 0.0f;
        timer_b += GetFrameTime();

        static int direction_a = -1;
        if (timer_b >= 0.1f) {
            timer_b = 0.0f;
            if (direction_a == -1) {
                tr -= 50;
                if (tr <= 0) {
                    tr = 0;
                    direction_a = 1;
                }
            } else {
                tr += 50;
                if (tr >= 255) {
                    tr = 255;
                    direction_a = -1;
                }
            }
        }
        static float timer_f = 0.0f;
        timer_f += GetFrameTime();
        if (timer_f >= 1) {
            timer_f = 0.0f;
            tr = 255;
            jon = h;
        }
    }
    Color semiTransparentWhite = (Color){255, 255, 255, tr};
    DrawTextureEx(PacMan[finalFrame], (Vector2){x, y}, 0.0f, 1, semiTransparentWhite);
}

void movement(int *i, int *j, int *currentDirection, int *star,
              int *timer_p, int *timer_q, int *lives, int *wait_f, int *wait_g) {
    int new_i = *i;
    int new_j = *j;
    static float frameTime = 0.2f;
    static float timer = 0.0f;
    static bool hasStarted = false;
    timer += GetFrameTime();
    if (!hasStarted && (IsKeyDown(KEY_UP) || IsKeyDown(KEY_DOWN) ||
                        IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT))) {
        hasStarted = true;
    }
    if (hasStarted) {
        if (IsKeyDown(KEY_RIGHT) && *j + 1 < MAZE_WIDTH && initialmaze[*i][*j + 1] != '#' && initialmaze[*i][*j + 1] !=
                                                                                             '_') {
            *currentDirection = 0;
        } else if (IsKeyDown(KEY_LEFT) && *j - 1 >= 0 && initialmaze[*i][*j - 1] != '#' && initialmaze[*i][*j - 1] !=
                                                                                           '_') {
            *currentDirection = 1;
        } else if (IsKeyDown(KEY_UP) && *i - 1 >= 0 && initialmaze[*i - 1][*j] != '#' && initialmaze[*i - 1][*j] !=
                                                                                         '_') {
            *currentDirection = 2;
        } else if (IsKeyDown(KEY_DOWN) && *i + 1 < MAZE_HEIGHT && initialmaze[*i + 1][*j] != '#' && initialmaze[*i + 1][
                                                                                                            *j] != '_') {
            *currentDirection = 3;
        }

        if (timer >= frameTime) {
            if (*currentDirection == 0 && *j + 1 < MAZE_WIDTH && initialmaze[*i][*j + 1] != '#' && initialmaze[*i][
                                                                                                           *j + 1] != '_') {
                new_j++;
            } else if (*currentDirection == 1 && *j - 1 >= 0 && initialmaze[*i][*j - 1] != '#' && initialmaze[*i][
                                                                                                          *j - 1] != '_') {
                new_j--;
            } else if (*currentDirection == 2 && *i - 1 >= 0 && initialmaze[*i - 1][*j] != '#' && initialmaze[*i - 1][*
                    j] != '_') {
                new_i--;
            } else if (*currentDirection == 3 && *i + 1 < MAZE_HEIGHT && initialmaze[*i + 1][*j] != '#' && initialmaze[
                                                                                                                   *i + 1][*j] != '_') {
                new_i++;
            }
            timer = 0.0f;
        }
        static float timer_f = 0.0f;
        timer_f += GetFrameTime();
        static int ff = 0;
        if (timer_f >= *wait_f && ff) {
            frameTime = 0.2f;
            timer_f = 0.0f;
            *wait_f = 0;
            f -= ff;
            ff = 0;
        }
        if (*lives < 6 && ch_a == 0) {
            a--;
        }
        ch_a = 0;
        *timer_p = *wait_f - timer_f;

        static float timer_g = 0.0f;
        timer_g += GetFrameTime();
        static int gg = 0;
        if (timer_g >= *wait_g && gg) {
            (*lives)--;
            timer_g = 0.0f;
            *wait_g = 0;
            g -= gg;
            gg = 0;
        }
        *timer_q = *wait_g - timer_g;
        if (new_i != *i || new_j != *j) {
            if (initialmaze[new_i][new_j] == 's') {
                C_s--;
                if (C_s <= 0) {
                    s = 0;
                    C_s = 10;
                }
                (*star)++;
            } else if (initialmaze[new_i][new_j] == 'f') {
                frameTime = 0.1f;
                *wait_f += 10;
                ff++;
                timer_f = 0;
            } else if (initialmaze[new_i][new_j] == 'a') {
                (*lives)++;
                jon++;
                a--;
            } else if (initialmaze[new_i][new_j] == 'm') {
                (*lives)--;
                jon--;
                m--;
            } else if (initialmaze[new_i][new_j] == 'g') {
                (*lives)++;
                jon++;
                gg++;
            }
            initialmaze[*i][*j] = ' ';
            initialmaze[new_i][new_j] = '@';
            *i = new_i;
            *j = new_j;
        }
    }
}

void toLowerCase(char *str) {
    while (*str) {
        *str = tolower((unsigned char) *str);
        str++;
    }
}

int GameOver(Texture2D deathAnim[11], Texture2D gameover, Color myColor, int i, int j, int score, char name[30],
             Font pacmanFont, int minutes, int seconds) {
    BeginDrawing();
    ClearBackground(myColor);
    static bool am_f = false;
    if (!am_f) {
        static int currentFrame = 0;
        static float frameTime = 0.2f;
        static float timer = 0.0f;

        timer += GetFrameTime();
        if (timer >= frameTime) {
            timer = 0.0f;
            if (currentFrame < 10) {
                currentFrame++;
            }
        }

        int x = j * tileSize + 70;
        int y = i * tileSize + 70;

        DrawTexture(deathAnim[currentFrame], x, y, WHITE);
        if (currentFrame >= 10) {
            am_f = true;
        }
    } else {
        static int choice = 0;
        Color tc[2] = {RAYWHITE, RAYWHITE};
        tc[choice] = GREEN;
        DrawTexture(gameover, 0, 0, WHITE);
        DrawText("Give it another shot!", 120, 630, 40, tc[0]);
        DrawText("Back to Main Menu", 140, 700, 40, tc[1]);
        if (IsKeyPressed(KEY_DOWN)) {
            choice = (choice + 1) % 2;
        }
        if (IsKeyPressed(KEY_UP)) {
            choice = (choice - 1 + 2) % 2;
        }
        if (IsKeyPressed(KEY_ENTER)) {
            EndDrawing();
            return choice;
        }
        toLowerCase(name);
        DrawTextEx(pacmanFont, name, (Vector2){230, 530}, 20, 2, RAYWHITE);
        DrawTextEx(pacmanFont, "score : ", (Vector2){500, 530}, 20, 2, RAYWHITE);
        char sc[20];
        sprintf(sc, "%d", score * 10);
        DrawText(sc, 630, 525, 30, RAYWHITE);
        char t[20];
        sprintf(t, "You have been playing for [%0.2d:%0.2d]. Keep it up!", minutes, seconds);
        DrawText(t, 355, 580, 15, RAYWHITE);
    }
    EndDrawing();
    return -1;
}

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
    strcpy(new_player.name, player_name);
    new_player.stars = star;
    new_player.total_time = minutes * 60 + seconds;
    time_t current_time;
    struct tm *local_time;
    char formatted_time[50];

    time(&current_time);
    local_time = localtime(&current_time);

    strftime(formatted_time, sizeof(formatted_time), "%Y/%m/%d", local_time);

    strcpy(new_player.time, formatted_time);
    players[player_count++] = new_player;

    qsort(players, player_count, sizeof(Player), comparePlayers);

    if (player_count > MAX_PLAYERS) {
        player_count = MAX_PLAYERS;
    }

    fp = fopen("saves.bin", "wb");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    fwrite(players, sizeof(Player), player_count, fp);
    fclose(fp);

    printf("Top 10 players saved successfully!\n");
    return EXIT_SUCCESS;
}

int maingame(void) {
    InitAudioDevice();
    Music backgroundMusic = LoadMusicStream("../assets/audio/take_on_me_small.mp3");
    PlayMusicStream(backgroundMusic);

    SetMusicVolume(backgroundMusic, 0.5f);
    SetMusicPitch(backgroundMusic, 1.0f);

    int r = -1;
    Font customFont = LoadFontEx("../assets/fonts/calibri.ttf", 32, NULL, 0);
    char player_name[30];
    getnames(player_name, 30, customFont);
    Font pacmanFont = LoadFont("../assets/fonts/PAC-FONT.TTF");

    resetMaze();

    Texture2D textures[5];
    textures[0] = LoadTexture("../assets/sprites/points/star.png");
    textures[1] = LoadTexture("../assets/sprites/points/cherry.png");
    textures[2] = LoadTexture("../assets/sprites/points/pepper.png");
    textures[3] = LoadTexture("../assets/sprites/points/apple.png");
    textures[4] = LoadTexture("../assets/sprites/points/mushroom.png");

    Texture2D PacMan[16];
    char *Direction[] = {"Right", "Left", "Up", "Down"};
    for (int n = 0; n < 4; n++) {
        for (int m = 0; m < 4; m++) {
            char location[128];
            sprintf(location, "../assets/sprites/pac/Direction/%s/%d.png", Direction[n], m + 1);
            PacMan[n * 4 + m] = LoadTexture(location);
        }
    }

    Texture2D ghost[40];
    char ghost_location[5][10] = {"blue", "green", "orange", "pink", "red"};
    char ghost_dir[8][3] = {"D1", "D2", "L1", "L2", "R1", "R2", "U1", "U2"};
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 8; j++) {
            char location[124];
            sprintf(location, "../assets/sprites/ghosts/%s/%s.png", ghost_location[i], ghost_dir[j]);
            ghost[i * 8 + j] = LoadTexture(location);
        }
    }

    Texture2D GHOST_BLUE[4];
    char *g_b[4] = {"blue1", "blue2", "white1", "white2"};
    for (int n = 0; n < 4; n++) {
        char location[100];
        sprintf(location, "../assets/sprites/ghosts/vulnerable/%s.png", g_b[n]);
        GHOST_BLUE[n] = LoadTexture(location);
    }

    Texture2D deathAnim[11];
    for (int i = 0; i < 11; i++) {
        char location[150];
        sprintf(location, "../assets/sprites/pac/deathAnim/death%d.png", i + 1);
        deathAnim[i] = LoadTexture(location);
    }

    Texture2D gameover = LoadTexture("../assets/sprites/GameOver.png");
    Texture2D Heart = LoadTexture("../assets/sprites/heart.png");

    RandomGhostPosition();

    Game game;
    initGame(&game);

    strncpy(game.player.name, player_name, NAME_SIZE - 1);
    game.player.name[NAME_SIZE - 1] = '\0';

    game.lives = 3;
    jon = 3;

    Color myColor = (Color){20, 25, 30, 255};
    SetTargetFPS(60);

    int ii = 0, jj = 0;
    int currentDirection = 0;
    int star = 0, timer_p = 0, timer_q = 0;
    int minutes = 0, seconds = 0;

    while (!WindowShouldClose()) {
        UpdateMusicStream(backgroundMusic);

        if (game.lives > 0) {
            BeginDrawing();
            ClearBackground(myColor);

            updateGame(&game);

            int count = map();
            insert(count, textures, &ii, &jj);

            currentDirection = game.player.direction;
            star = game.player.stars;
            timer_p = (int)game.player.powerTimer;
            timer_q = (int)game.ghostTimer;

            can_eat = game.canEatGhosts;

            pacman(PacMan, ii, jj, currentDirection, game.lives);
            score(star, timer_p, timer_q);
            heart(game.lives, Heart);
            print_Name_Time(player_name, customFont, &minutes, &seconds);

            if (game.canEatGhosts) {
                eat_ghost(GHOST_BLUE, ii, jj, &star, &timer_q);
            } else {
                insert_ghost(ghost, ii, jj, &game.lives);
            }

            game.player.stars = star;
            game.score = star * 10;
            game.player.total_time = minutes * 60 + seconds;

            jon = game.lives;

            EndDrawing();
        } else {
            int ch = GameOver(deathAnim, gameover, myColor, ii, jj, star, player_name, pacmanFont, minutes, seconds);
            if (ch == 0) {
                r = 0;
                break;
            }
            if (ch == 1) {
                r = 1;
                break;
            }
        }
    }

    save_player(player_name, star * 10, minutes, seconds);

    unloadGameResources(textures, PacMan, ghost, GHOST_BLUE, deathAnim, gameover, Heart, pacmanFont, customFont, backgroundMusic);
    CloseAudioDevice();

    return r;
}

void initGame(Game *game) {
    resetMaze();

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            game->maze[i][j] = initialmaze[i][j];

            if (initialmaze[i][j] == '@') {
                game->player.position = (Vector2){ j * TILE_SIZE, i * TILE_SIZE };
            }
        }
    }

    game->player.speed = PACMAN_SPEED;
    game->player.direction = 0;
    game->player.isPowered = false;
    game->player.powerTimer = 0;
    game->player.score = 0;
    game->player.stars = 0;
    game->player.total_time = 0;

    game->state = GAME_STATE_PLAYING;
    game->score = 0;
    game->gameTime = 0;
    game->canEatGhosts = false;
    game->ghostTimer = 0;
    game->lives = 3;

    RandomGhostPosition();
    initGhostManager(&game->ghostManager);

    jon = game->lives;
    can_eat = game->canEatGhosts;
}

void updateGame(Game *game) {
    float deltaTime = GetFrameTime();

    if (game->canEatGhosts) {
        game->ghostTimer -= deltaTime;
        if (game->ghostTimer <= 0) {
            game->canEatGhosts = false;
            for (int i = 0; i < GHOST_COUNT; i++) {
                game->ghostManager.ghosts[i].state = GHOST_STATE_CHASE;
            }
        }
    }

    // FIX 7: فقط یکبار updateGhostStates و updateGhosts صدا زده می‌شن
    //         قبلا دو بار صدا زده می‌شد و ghost ها دو برابر سریع حرکت می‌کردن
    updateGhostStates(&game->ghostManager, deltaTime);
    updateGhosts(&game->ghostManager, game->player.position, deltaTime);

    handleInput(game);

    if (game->player.isPowered) {
        game->player.powerTimer -= deltaTime;
        if (game->player.powerTimer <= 0) {
            game->player.isPowered = false;
            game->canEatGhosts = false;
            can_eat = false;
        }
    }

    if (game->canEatGhosts) {
        for (int i = 0; i < GHOST_COUNT; i++) {
            game->ghostManager.ghosts[i].state = GHOST_STATE_FRIGHTENED;
        }
    }

    checkCollisions(game);

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            if (game->maze[i][j] == '@') {
                game->maze[i][j] = ' ';
            }
        }
    }

    int playerTileX = (int)(game->player.position.x / TILE_SIZE);
    int playerTileY = (int)(game->player.position.y / TILE_SIZE);
    game->maze[playerTileY][playerTileX] = '@';
}

void handleInput(Game *game) {
    int timer_p = 0, timer_q = 0;

    int i_p = (int)(game->player.position.y / TILE_SIZE);
    int j_p = (int)(game->player.position.x / TILE_SIZE);
    int curr_direction = game->player.direction;
    int stars = game->player.stars;
    int lives = game->lives;

    // FIX 8: wait_f و wait_g باید static باشن تا بین فریم‌ها حفظ بشن
    //         قبلا هر فریم صفر می‌شدن و power-up های سرعت و ghost هیچ‌وقت کار نمی‌کردن
    static int wait_f = 0;
    static int wait_g = 0;

    movement(&i_p, &j_p, &curr_direction, &stars,
             &timer_p, &timer_q, &lives, &wait_f, &wait_g);

    game->player.position.y = i_p * TILE_SIZE;
    game->player.position.x = j_p * TILE_SIZE;
    game->player.direction = curr_direction;
    game->player.stars = stars;
    game->player.powerTimer = (float)timer_p;
    game->ghostTimer = (float)timer_q;
    game->lives = lives;

    jon = game->lives;
    can_eat = game->canEatGhosts;
}

void checkCollisions(Game *game) {
    Rectangle playerRect = {
            game->player.position.x,
            game->player.position.y,
            TILE_SIZE, TILE_SIZE
    };

    for (int i = 0; i < GHOST_COUNT; i++) {
        Rectangle ghostRect = {
                game->ghostManager.ghosts[i].position.x,
                game->ghostManager.ghosts[i].position.y,
                TILE_SIZE, TILE_SIZE
        };

        if (CheckCollisionRecs(playerRect, ghostRect)) {
            if (game->canEatGhosts) {
                game->score += 200;
                game->ghostManager.ghosts[i].isEaten = true;
                game->ghostManager.ghosts[i].position = getGhostSpawnPosition(i);
            } else {
                game->lives--;
                game->player.position = getPlayerStartPosition();
                resetGhostPositions(game);
            }
        }
    }
}

void drawGame(Game *game) {
    drawMaze(game);
    drawPlayer(game);
    drawGhostsVisual(&game->ghostManager);
    drawScore(game);
    drawTimer(game);
}

void drawMaze(Game *game) {
    Color MY_BLUE = (Color){50, 120, 140, 255};
    const int borderThickness = 4;

    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            int x = j * TILE_SIZE + 70;
            int y = i * TILE_SIZE + 70;

            switch (game->maze[i][j]) {
                case '#':
                    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, MY_BLUE);
                    break;
                case '_':
                    DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, (Color){20, 15, 85, 255});
                    DrawRectangle(x + borderThickness, y + borderThickness,
                                  TILE_SIZE - 2 * borderThickness,
                                  TILE_SIZE - 2 * borderThickness,
                                  (Color){50, 120, 140, 150});
                    break;
                case 's':
                    DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/4, YELLOW);
                    break;
                case 'g':
                    DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/3, BLUE);
                    break;
                case 'f':
                    DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/3, RED);
                    break;
                case 'a':
                    DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/3, GREEN);
                    break;
                case 'm':
                    DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, TILE_SIZE/3, PURPLE);
                    break;
            }
        }
    }
}

void drawPlayer(Game *game) {
    (void)game;
}

void drawScore(Game *game) {
    DrawText(TextFormat("Score: %d", game->score), 20, 20, 20, WHITE);
}

void drawTimer(Game *game) {
    int minutes = (int)game->gameTime / 60;
    int seconds = (int)game->gameTime % 60;
    DrawText(TextFormat("Time: %02d:%02d", minutes, seconds), 550, 20, 20, WHITE);
}

void resetMaze(void) {
    for (int i = 0; i < MAZE_HEIGHT; i++) {
        for (int j = 0; j < MAZE_WIDTH; j++) {
            initialmaze[i][j] = initialmaze_I[i][j];
        }
    }

    s = 0;
    g = 0;
    f = 0;
    a = 0;
    m = 0;
    ch_a = 0;
    C_s = 10;
    jon = 3;
    can_eat = false;
}

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

void drawGhostsVisual(GhostManager *manager) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        if (!manager->ghosts[i].isEaten) {
            Color ghostColor = manager->ghosts[i].color;
            if (manager->ghosts[i].state == GHOST_STATE_FRIGHTENED) {
                ghostColor = BLUE;
            }
            DrawCircleV(manager->ghosts[i].position, TILE_SIZE/2, ghostColor);
        }
    }
}

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

void resetGhostPositions(Game *game) {
    for (int i = 0; i < GHOST_COUNT; i++) {
        game->ghostManager.ghosts[i].position = getGhostSpawnPosition(i);
        game->ghostManager.ghosts[i].isEaten = false;
    }
}

void checkPowerPellet(Game *game) {
    int tileX = (int)(game->player.position.x / TILE_SIZE);
    int tileY = (int)(game->player.position.y / TILE_SIZE);

    if (game->maze[tileY][tileX] == 'g') {
        game->canEatGhosts = true;
        game->ghostTimer = POWER_PELLET_DURATION;
        game->maze[tileY][tileX] = ' ';

        for (int i = 0; i < GHOST_COUNT; i++) {
            game->ghostManager.ghosts[i].state = GHOST_STATE_FRIGHTENED;
        }
    }
}