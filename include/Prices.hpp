#pragma once

#include <string>
#include <vector>

struct DealEntry
{
    std::string name;
    double price;

    DealEntry() {}
    DealEntry(std::string name, double price) : name(name), price(price) {}
};

inline bool operator<(const DealEntry& a, const DealEntry& b)
{
    return a.price < b.price;
}

std::string GetEbayToken(bool sandbox = false);
std::vector<DealEntry> GetEbayDeals(const std::string& query, const std::string& token, bool sandbox = false);
