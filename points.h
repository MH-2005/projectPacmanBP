#pragma once   // FIX: اضافه شد

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

void score(int star, int timer_p, int timer_q) {
    char sc[30];
    sprintf(sc, "Score: %d", star * 10);
    DrawText(sc, 70, 20, 40, WHITE);
    DrawRectangle(70, 700, timer_p * 15, 30, RED);
    DrawRectangle(630 - timer_q * 15, 700, timer_q * 15, 30, BLUE);
}

void heart(int h, Texture2D Heart) {
    for (int i = 0; i < h; i++) {
        DrawTexture(Heart, 590 - i * 50, 20, WHITE);
    }
}

void print_Name_Time(char name[30], Font customFont, int *minutes, int *seconds) {
    DrawTextEx(customFont, name, (Vector2){70, 750}, 20, 2, WHITE);

    static float elapsed_time = 0.0f;
    elapsed_time += GetFrameTime();

    *minutes = (int) elapsed_time / 60;
    *seconds = (int) elapsed_time % 60;

    char timeText[30];
    snprintf(timeText, sizeof(timeText), "Time: %02d:%02d", *minutes, *seconds);

    DrawText(timeText, 500, 750, 20, WHITE);
}