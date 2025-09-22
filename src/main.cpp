#include "Settings.hpp"
#include "UI.hpp"
#include <raygui.h>

int main()
{
    Load();

    // Set raylib config flags
    int flags = 0;
    flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);

    // Init raylib
    InitWindow(windowSize.x, windowSize.y, "PC Best Deals");
    SetExitKey(-1);

    GuiSetFont(GetFontDefault());

    // Main loop
    while (!WindowShouldClose())
    {
        DrawFrame();
    }

    // Close the program
    Save();
    CloseWindow();

    return 0;
}
