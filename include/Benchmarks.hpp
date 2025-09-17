#pragma once

#include <ostream>
#include <string_view>

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
};

inline bool operator<(const BenchmarkEntry& a, const BenchmarkEntry& b)
{
    if (a.type != b.type) return a.type < b.type;
    if (a.sceneName != b.sceneName) return a.sceneName < b.sceneName;
    if (a.name != b.name) return a.name < b.name;
    return a.score < b.score;
}
void DownloadBlenderBenchmarks();
void ProcessBlenderBenchmarks();
