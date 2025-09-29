// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <ostream>
#include <string>
#include <vector>

#define BENCHMARKS_PATH "tmp/benchmarks.csv"

enum class DeviceType
{
    CPU,
    GPU
};

inline std::ostream& operator<<(std::ostream& out, const DeviceType& deviceType)
{
    return out << (deviceType == DeviceType::CPU ? "CPU" : "GPU");
}

struct BenchmarkEntry
{
    std::string name, sceneName;
    DeviceType type;
    double score;

    BenchmarkEntry() {}
    BenchmarkEntry(std::string name, std::string sceneName, DeviceType type, double score)
        : name(name), sceneName(sceneName), type(type), score(score)
    {
    }
    BenchmarkEntry(std::string name, DeviceType type, double score)
        : name(name), type(type), score(score)
    {
    }
};

inline bool operator<(const BenchmarkEntry& a, const BenchmarkEntry& b)
{
    if (a.type != b.type) return a.type < b.type;
    if (a.name != b.name) return a.name < b.name;
    if (a.sceneName != b.sceneName) return a.sceneName < b.sceneName;
    return a.score < b.score;
}

extern std::string blenderBenchmarksDownloadStatus;

void DownloadBlenderBenchmarks();
std::vector<BenchmarkEntry> ProcessBlenderBenchmarks();
std::vector<BenchmarkEntry> GetBlenderBenchmarks();
