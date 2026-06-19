#pragma once   // FIX 1: اضافه شد

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "mainGame.h"   // FIX 2: اضافه شد - بدون این Player و comparePlayers شناخته نمی‌شدن

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
    char displayLines[MAX_PLAYERS][100];
    float alpha = 0.0f;

    while (!WindowShouldClose()) {
        alpha = Clamp(alpha + GetFrameTime(), 0.0f, 1.0f);

        BeginDrawing();
        ClearBackground(backgroundColor);
        DrawTexture(scores, 0, 0, WHITE);

        FILE *fp = fopen("saves.bin", "rb");
        Player players[MAX_PLAYERS];
        int player_count = 0;

        if (fp != NULL) {
            while (fread(&players[player_count], sizeof(Player), 1, fp) == 1) {
                int minutes = players[player_count].total_time / 60;
                int seconds = players[player_count].total_time % 60;

                snprintf(displayLines[player_count], 100,
                         "%-20s SCORE: %2d  TIME: %02d:%02d Date : %s",
                         players[player_count].name,
                         players[player_count].stars,
                         minutes,
                         seconds,
                         players[player_count].time);

                player_count++;
                if (player_count >= MAX_PLAYERS) break;
            }
            fclose(fp);
        }

        qsort(players, player_count, sizeof(Player), (int (*)(const void*, const void*))comparePlayers);

        int displayCount = player_count > 10 ? 10 : player_count;
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