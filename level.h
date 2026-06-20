#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

static int current_level = 1;

int get_game_level(void) {
    return current_level;
}

void choose_level(void) {
    Color backgroundColor = (Color){20, 25, 30, 255};
    const char *levels[] = {"EASY", "NORMAL", "HARD"};
    int choice = current_level - 1;
    if (choice < 0 || choice > 2) choice = 0;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(backgroundColor);

        DrawText("SELECT LEVEL", 230, 200, 50, YELLOW);

        Color tc[3] = {RAYWHITE, RAYWHITE, RAYWHITE};
        tc[choice] = GREEN;
        for (int i = 0; i < 3; i++) {
            DrawText(levels[i], 290, 320 + i * 70, 40, tc[i]);
        }

        DrawText("UP/DOWN to select, ENTER to confirm", 150, 600, 20, LIGHTGRAY);

        EndDrawing();

        if (IsKeyPressed(KEY_DOWN)) {
            choice = (choice + 1) % 3;
        }
        if (IsKeyPressed(KEY_UP)) {
            choice = (choice - 1 + 3) % 3;
        }
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            current_level = choice + 1;
            break;
        }
    }
}
