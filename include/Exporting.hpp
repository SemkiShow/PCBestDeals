// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Benchmarks.hpp"
#include "Prices.hpp"
#include <unordered_map>

void ExportAsXlsx(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks,
                  const std::unordered_map<std::string, DealEntry>& prices);
