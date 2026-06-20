#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "mainGame.h"

#define MAX_PLAYERS 10

void show_score(void) {
    Color backgroundColor = (Color){20, 25, 30, 255};
    Color textColors[10] = {
        (Color){255, 215, 0, 255},
        (Color){192, 192, 192, 255},
        (Color){205, 127, 50, 255},
        WHITE, WHITE, WHITE, WHITE, WHITE, WHITE, WHITE
    };

    Texture2D scores = LoadTexture("../assets/sprites/scores.png");
    char displayLines[MAX_PLAYERS][150];
    Player players[MAX_PLAYERS];
    int player_count = 0;
    float alpha = 0.0f;

    FILE *fp = fopen("saves.bin", "rb");
    if (fp != NULL) {
        while (fread(&players[player_count], sizeof(Player), 1, fp) == 1) {
            player_count++;
            if (player_count >= MAX_PLAYERS) break;
        }
        fclose(fp);
    }

    qsort(players, player_count, sizeof(Player),
          (int (*)(const void *, const void *)) comparePlayers);

    for (int i = 0; i < player_count; i++) {
        int minutes = players[i].total_time / 60;
        int seconds = players[i].total_time % 60;
        snprintf(displayLines[i], sizeof(displayLines[i]),
                 "%-20s SCORE: %4d  TIME: %02d:%02d Date: %s",
                 players[i].name, players[i].stars,
                 minutes, seconds, players[i].time);
    }

    while (!WindowShouldClose()) {
        alpha = Clamp(alpha + GetFrameTime(), 0.0f, 1.0f);

        BeginDrawing();
        ClearBackground(backgroundColor);
        DrawTexture(scores, 0, 0, WHITE);

        int displayCount = player_count > MAX_PLAYERS ? MAX_PLAYERS : player_count;
        for (int i = 0; i < displayCount; i++) {
            Color color = textColors[i];
            color.a = (unsigned char) (255 * alpha);
            DrawText(displayLines[i], 80, 285 + i * 50, 10, color);
        }

        DrawText("Press SPACE to return",
                 GetScreenWidth() / 2 - MeasureText("Press SPACE to return", 20) / 2,
                 GetScreenHeight() - 25, 15, WHITE);

        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {
            break;
        }
    }

    UnloadTexture(scores);
}
