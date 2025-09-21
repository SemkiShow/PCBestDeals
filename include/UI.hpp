#pragma once

#include "Benchmarks.hpp"
#include "Prices.hpp"
#include <raylib.h>
#include <vector>

extern Vector2 windowSize;
extern std::vector<BenchmarkEntry> benchmarks;
extern std::vector<DealEntry> prices;

void DrawFrame();
