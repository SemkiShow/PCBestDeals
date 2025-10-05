// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <ostream>
#include <string>
#include <unordered_map>

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
    std::string name, scene, version;
    DeviceType type;
    double score;

    BenchmarkEntry() {}
    BenchmarkEntry(std::string name, std::string scene, DeviceType type, double score)
        : name(name), scene(scene), type(type), score(score)
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
    if (a.scene != b.scene) return a.scene < b.scene;
    return a.score < b.score;
}

extern std::string blenderBenchmarksDownloadStatus;

void DownloadBlenderBenchmarks();
std::unordered_map<std::string, BenchmarkEntry> ProcessBlenderBenchmarks();
std::unordered_map<std::string, BenchmarkEntry> GetBlenderBenchmarks();
