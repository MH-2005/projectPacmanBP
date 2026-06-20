#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

static int menu_fade = 0;

int menu(Texture2D image, Sound sound) {
    Color semiTransparentWhite = (Color){255, 255, 255, menu_fade};
    DrawTextureEx(image, (Vector2){55, 50}, 0.0f, 0.2f, semiTransparentWhite);

    if (menu_fade < 250) {
        menu_fade += 5;
        return -1;
    }

    static int choice = 0;

    const char *lines[] = {"START GAME", "LEVEL", "MY SCORES", "EXIT"};
    Color tc[4] = {RAYWHITE, RAYWHITE, RAYWHITE, RAYWHITE};
    tc[choice] = GREEN;

    for (int i = 0; i < 4; i++) {
        DrawText(lines[i], 150, 300 + i * 85, 40, tc[i]);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        PlaySound(sound);
        choice = (choice + 1) % 4;
    }
    if (IsKeyPressed(KEY_UP)) {
        PlaySound(sound);
        choice = (choice - 1 + 4) % 4;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        return choice;
    }

    return -1;
}

int Begin(void) {
    menu_fade = 0;

    Color myColor = (Color){20, 25, 30, 255};
    Texture2D image = LoadTexture("../assets/sprites/pngegg.png");
    Sound sound = LoadSound("../assets/audio/credit.wav");
    Sound click = LoadSound("../assets/audio/munch_1.wav");
    int result = -1;

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(myColor);

        result = menu(image, sound);

        if (result >= 0) {
            PlaySound(click);
            EndDrawing();
            break;
        }

        EndDrawing();
    }

    UnloadTexture(image);
    UnloadSound(sound);
    UnloadSound(click);

    return result;
}
