#include "UI.hpp"
#include "raygui.h"
#include "raylib.h"

int main()
{
    // Set raylib config flags
    int flags = 0;
    flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

    // Init raylib
    InitWindow(windowSize[0], windowSize[1], "PC Best Deals");
    SetExitKey(-1);

    // Main loop
    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    // Close the program
    CloseWindow();

    return 0;
}
