#include "Benchmarks.hpp"
#include "Exporting.hpp"
#include "Prices.hpp"
#include "UI.hpp"
#include <raygui.h>

int main()
{
    // Set raylib config flags
    int flags = 0;
    flags |= FLAG_VSYNC_HINT;
    flags |= FLAG_WINDOW_HIGHDPI;
    flags |= FLAG_WINDOW_RESIZABLE;
    SetConfigFlags(flags);
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
    auto benchmarks = GetBlenderBenchmarks();
    auto token = GetEbayToken();
    auto partPrices = GetEbayPartPrices(benchmarks, token);
    ExportAsXlsx(benchmarks, partPrices);
    return 0;

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
    CloseWindow();

    return 0;
}
