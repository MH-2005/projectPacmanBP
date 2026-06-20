#pragma once

#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <math.h>

#define FONT_SIZE 30
#define PADDING 10
#define MAX_NAME_LENGTH 30

void getnames(char *outputName, size_t outputSize, Font customFont) {
    const Color BACKGROUND_COLOR = (Color){45, 45, 45, 255};
    const Color TEXT_COLOR = WHITE;
    const Color INPUT_FIELD_COLOR = (Color){60, 60, 60, 255};
    const Color ACTIVE_INPUT_COLOR = (Color){75, 75, 75, 255};
    const Color BORDER_COLOR = SKYBLUE;
    char playerName[MAX_NAME_LENGTH] = {0};
    int nameLength = 0;
    bool inputPhase = true;
    bool isInputActive = true;
    float showWelcomeTime = 0.0f;

    Rectangle inputBox = {
        GetScreenWidth() / 2.0f - 250,
        GetScreenHeight() / 2.0f + FONT_SIZE / 2.0f - 10,
        500,
        FONT_SIZE + 4 * PADDING
    };

    while (!WindowShouldClose()) {
        if (inputPhase && isInputActive) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (nameLength < MAX_NAME_LENGTH - 1)) {
                    float textW = MeasureTextEx(customFont, playerName, FONT_SIZE, 2).x;
                    if (textW < inputBox.width - 2 * PADDING - FONT_SIZE) {
                        playerName[nameLength] = (char) key;
                        nameLength++;
                        playerName[nameLength] = '\0';
                    }
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                if (nameLength > 0) {
                    nameLength--;
                    playerName[nameLength] = '\0';
                }
            } else if (IsKeyPressed(KEY_ENTER) && nameLength > 0) {
                inputPhase = false;
                showWelcomeTime = GetTime();
                strncpy(outputName, playerName, outputSize - 1);
                outputName[outputSize - 1] = '\0';
            }
        }

        BeginDrawing(); {
            ClearBackground(BACKGROUND_COLOR);

            if (inputPhase) {
                const char *prompt = "Enter your name:";
                Vector2 promptPos = {
                    GetScreenWidth() / 2.0f - MeasureTextEx(customFont, prompt, FONT_SIZE, 2).x / 2.0f,
                    GetScreenHeight() / 2.0f - FONT_SIZE - PADDING - 20
                };
                DrawTextEx(customFont, prompt, (Vector2){promptPos.x, promptPos.y}, FONT_SIZE, 2, TEXT_COLOR);

                DrawRectangleRec(inputBox, isInputActive ? ACTIVE_INPUT_COLOR : INPUT_FIELD_COLOR);
                DrawRectangleLinesEx(inputBox, 2, BORDER_COLOR);

                Vector2 textPos = {
                    inputBox.x + PADDING,
                    inputBox.y + (inputBox.height - FONT_SIZE) / 2
                };
                DrawTextEx(customFont, playerName, (Vector2){textPos.x, textPos.y}, FONT_SIZE, 2, TEXT_COLOR);

                float cursorOffset = MeasureTextEx(customFont, playerName, FONT_SIZE, 2).x;
                if (isInputActive && (fmod(GetTime() * 2, 2) < 1)) {
                    DrawTextEx(customFont, "_", (Vector2){textPos.x + cursorOffset, textPos.y},
                               FONT_SIZE, 2, TEXT_COLOR);
                }
            } else {
                char welcomeText[100];
                snprintf(welcomeText, sizeof(welcomeText), "Welcome, %s!", playerName);
                Vector2 welcomePos = {
                    GetScreenWidth() / 2.0f - MeasureTextEx(customFont, welcomeText, FONT_SIZE, 2).x / 2.0f,
                    GetScreenHeight() / 2.0f - FONT_SIZE / 2.0f
                };
                DrawTextEx(customFont, welcomeText, (Vector2){welcomePos.x, welcomePos.y}, FONT_SIZE, 2, TEXT_COLOR);

                if (GetTime() - showWelcomeTime > 0.5f) {
                    break;
                }
            }
        }
        EndDrawing();
    }
}
