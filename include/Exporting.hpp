// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Benchmarks.hpp"
#include "Prices.hpp"
#include <vector>

void ExportAsXlsx(const std::vector<BenchmarkEntry>& benchmarks,
                  const std::vector<DealEntry>& prices);
