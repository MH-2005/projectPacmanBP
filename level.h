#pragma once   // FIX: اضافه شد

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

void choose_level(void) {
    Color backgroundColor = (Color){20, 25, 30, 255};

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(backgroundColor);
        EndDrawing();

        if (IsKeyPressed(KEY_SPACE)) {
            break;
        }
    }
}