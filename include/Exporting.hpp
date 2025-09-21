#pragma once

#include "Benchmarks.hpp"
#include "Prices.hpp"
#include <vector>

void ExportAsXlsx(const std::vector<BenchmarkEntry>& benchmarks,
                  const std::vector<DealEntry>& prices);
