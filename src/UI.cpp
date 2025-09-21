#include "UI.hpp"
#include <iostream>
#include <raygui.h>

Vector2 windowSize = {16 * 50 * 2, 9 * 50 * 2};

Vector2 GetCurrentWindowSize() { return {GetRenderWidth() * 1.f, GetRenderHeight() * 1.f}; }

bool DrawButton(Rectangle bounds, const char* text)
{
    auto window = GetCurrentWindowSize();
    bounds.x *= window.x / windowSize.x;
    bounds.y *= window.y / windowSize.y;
    bounds.width *= window.x / windowSize.x;
    bounds.height *= window.y / windowSize.y;
    return GuiButton(bounds, text);
}

void DrawFrame()
{
    BeginDrawing();

    ClearBackground(BLACK);

    if (DrawButton({20, 10, 200, 100}, "test")) std::cout << "test\n";

    EndDrawing();
}
