// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Benchmarks.hpp"
#include <string>
#include <vector>

#define PRICES_PATH "tmp/prices.csv"

struct DealEntry
{
    std::string name;
    double price;

    DealEntry() {}
    DealEntry(std::string name, double price) : name(name), price(price) {}
};

inline bool operator<(const DealEntry& a, const DealEntry& b) { return a.price < b.price; }

extern std::string ebayPricesDownloadStatus;

std::string GetEbayToken(bool sandbox = false);
std::vector<DealEntry> GetEbayDeals(const std::string& query, const std::string& token,
                                    bool recursive = false, bool sandbox = false, int offset = 0);
bool IsPricesDownloadComplete(const std::vector<BenchmarkEntry>& benchmarks, std::vector<DealEntry>& partPrices);
std::vector<DealEntry> DownloadEbayPartPrices(const std::vector<BenchmarkEntry>& benchmarks,
                                              const std::string& token, bool recursive = false,
                                              bool sandbox = false);
std::vector<DealEntry> GetEbayPartPrices(const std::vector<BenchmarkEntry>& benchmarks,
                                         const std::string& token, bool sandbox = false);
