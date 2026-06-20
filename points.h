#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

static float game_elapsed_time = 0.0f;

void reset_elapsed_time(void) {
    game_elapsed_time = 0.0f;
}

void draw_score(int star, int timer_p, int timer_q) {
    char sc[30];
    snprintf(sc, sizeof(sc), "Score: %d", star * 10);
    DrawText(sc, 70, 20, 40, WHITE);

    if (timer_p > 0) {
        int bar_p = timer_p * 15;
        if (bar_p > 560) bar_p = 560;
        DrawRectangle(70, 700, bar_p, 30, RED);
    }
    if (timer_q > 0) {
        int bar_q = timer_q * 15;
        if (bar_q > 630) bar_q = 630;
        DrawRectangle(630 - bar_q, 700, bar_q, 30, BLUE);
    }
}

void heart(int h, Texture2D Heart) {
    int max_hearts = 11;
    if (h > max_hearts) h = max_hearts;
    for (int i = 0; i < h; i++) {
        DrawTexture(Heart, 590 - i * 50, 20, WHITE);
    }
}

void print_Name_Time(char name[30], Font customFont, int *minutes, int *seconds) {
    DrawTextEx(customFont, name, (Vector2){70, 750}, 20, 2, WHITE);

    game_elapsed_time += GetFrameTime();

    *minutes = (int) game_elapsed_time / 60;
    *seconds = (int) game_elapsed_time % 60;

    char timeText[30];
    snprintf(timeText, sizeof(timeText), "Time: %02d:%02d", *minutes, *seconds);
    DrawText(timeText, 500, 750, 20, WHITE);
}
