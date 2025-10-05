// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Exporting.hpp"
#include "Settings.hpp"
#include "System.hpp"
#include "Utils.hpp"
#include "UI.hpp"
#include <filesystem>
#include <raygui.h>
#include <thread>

Vector2 windowSize = {16 * 50 * 2, 9 * 50 * 2};
int defaultFontSize = 24, fontSize = 24;

std::vector<BenchmarkEntry> benchmarks;
std::vector<DealEntry> prices;
bool ebayCredentialsExist = std::filesystem::exists("credentials.txt");
bool xlsxExportComplete = false;

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

void DrawTextScaled(const char* text, int posX, int posY, float fontSizeScale = 1.0,
                    Color color = {255, 255, 255, 255})
{
    auto window = GetCurrentWindowSize();
    posX *= window.x / windowSize.x;
    posY *= window.y / windowSize.y;
    DrawText(text, posX, posY, fontSize * fontSizeScale, color);
}

void DrawFrame()
{
    BeginDrawing();

    ClearBackground(BLACK);
    auto window = GetCurrentWindowSize();
    fontSize = std::min(defaultFontSize * window.x / windowSize.x,
                        defaultFontSize * window.y / windowSize.y);
    GuiSetStyle(DEFAULT, TEXT_SIZE, fontSize);

    int posY = 0;
    DrawTextScaled("Benchmark Data", 10, posY++ * defaultFontSize * 4, 4);
    DrawTextScaled("Blender benchmarks", 50, posY * defaultFontSize * 4, 2);
    if (benchmarksAvailable)
    {
        DrawTextScaled("Available", 550, posY++ * defaultFontSize * 4, 2, {0, 255, 0, 255});
    }
    else
    {
        if (blenderBenchmarksDownloadStatus == "")
        {
            if (DrawButton({550, float(posY++ * defaultFontSize * 4), 400, 50},
                           "Download Blender benchmarks"))
            {
                std::thread(GetBlenderBenchmarks).detach();
            }
        }
        else
        {
            DrawTextScaled(blenderBenchmarksDownloadStatus.c_str(), 550,
                           posY++ * defaultFontSize * 4, 2);
        }
    }

    DrawTextScaled("Price Data", 10, posY++ * defaultFontSize * 4, 4);
    DrawTextScaled("Ebay prices", 50, posY * defaultFontSize * 4, 2);
    if (pricesAvailable)
    {
        DrawTextScaled("Available", 550, posY++ * defaultFontSize * 4, 2, {0, 255, 0, 255});
    }
    else
    {
        if (!ebayCredentialsExist)
        {
            DrawTextScaled("You must enter your ebay developer\ncredentials! (see README.md)", 550,
                           posY++ * defaultFontSize * 4, 1.9, {255, 0, 0, 255});
        }
        else if (!benchmarksAvailable)
        {
            DrawTextScaled("You must download Blender benchmarks first!", 550,
                           posY++ * defaultFontSize * 4, 1.9, {255, 0, 0, 255});
        }
        else if (ebayPricesDownloadStatus == "")
        {
            if (DrawButton({550, float(posY++ * defaultFontSize * 4), 400, 50},
                           "Download Ebay prices"))
            {
                auto benchmarks = GetBlenderBenchmarks();
                auto token = GetEbayToken();
                std::thread([=] { DownloadEbayPartPrices(benchmarks, token); }).detach();
            }
        }
        else
        {
            DrawTextScaled(ebayPricesDownloadStatus.c_str(), 550, posY++ * defaultFontSize * 4, 2);
        }
    }

    if (benchmarksAvailable && pricesAvailable)
    {
        if (DrawButton({10, float(posY * defaultFontSize * 4), 400, 50}, "Export data as xlsx"))
        {
            auto benchmarks = GetBlenderBenchmarks();
            auto token = GetEbayToken();
            auto prices = GetEbayPartPrices(benchmarks, token);
            FilterBenchmarks(benchmarks);
            FilterPrices(prices);
            ExportAsXlsx(benchmarks, prices);
            xlsxExportComplete = true;
            OpenInFileManager(".");
        }
        if (xlsxExportComplete)
        {
            DrawTextScaled("Export complete!", 550, posY++ * defaultFontSize * 4, 2,
                           {0, 255, 0, 255});
        }
    }

    EndDrawing();
}
