#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#include "menu.h"
#include "mainGame.h"
#include "scores.h"
#include "level.h"

int main(void) {
    InitWindow(700, 800, "Pac-Man");
    SetTargetFPS(60);
    InitAudioDevice();

    while (!WindowShouldClose()) {
        int ch = Begin();
        switch (ch) {
            case 0:
                ch = maingame();
                break;
            case 1:
                choose_level();
                break;
            case 2:
                show_score();
                break;
            case 3:
                CloseAudioDevice();
                CloseWindow();
                return 0;
            default:
                break;
        }
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
