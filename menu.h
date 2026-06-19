#pragma once   // FIX: اضافه شد

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"

int menu(Texture2D image, Sound sound) {
    static int x = 0;
    Color semiTransparentWhite = (Color){255, 255, 255, x};
    DrawTextureEx(image, (Vector2){55, 50}, 0.0f, 0.2f, semiTransparentWhite);
    if (x < 250) {
        x += 5;
    } else {
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
    }
    return -1;
}

int Begin(void) {
    Color myColor = (Color){20, 25, 30, 255};
    SetTargetFPS(60);
    int result;
    Texture2D image = LoadTexture("../assets/sprites/pngegg.png");
    InitAudioDevice();
    Sound sound = LoadSound("../assets/audio/credit.wav");
    Sound click = LoadSound("../assets/audio/munch_1.wav");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(myColor);
        result = menu(image, sound);
        if (IsKeyDown(KEY_ENTER)) {
            PlaySound(click);
            break;
        }

        EndDrawing();
    }

    UnloadTexture(image);
    UnloadSound(sound);
    UnloadSound(click);
    CloseAudioDevice();
    return result;
}