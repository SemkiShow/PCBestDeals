// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Benchmarks.hpp"
#include "Prices.hpp"
#include <string>
#include <vector>

std::vector<std::string> Split(const std::string& input, const char delimiter = ',');
std::string TrimJunk(const std::string& input);
int DownloadFile(const std::string& url, const std::string& outputPath);
void WriteBenchmarks(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks);
void WritePrices(const std::unordered_map<std::string, DealEntry>& prices);
void FilterBenchmarks(std::unordered_map<std::string, BenchmarkEntry>& benchmarks);
void FilterData(std::unordered_map<std::string, BenchmarkEntry>& benchmarks, std::unordered_map<std::string, DealEntry>& prices);
