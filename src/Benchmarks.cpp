// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Benchmarks.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <simdjson.h>
#include <string>
#include <unordered_map>
#include <zip.h>

std::string blenderBenchmarksDownloadStatus = "";

int UnzipFile(const std::string& zipPath, const std::string& extractDir)
{
    int err = 0;
    zip* archive = zip_open(zipPath.c_str(), ZIP_RDONLY, &err);
    if (!archive)
    {
        std::cerr << "Failed to open zip archive: " << zipPath << '\n';
        return 1;
    }

    zip_int64_t numEntries = zip_get_num_entries(archive, 0);

    for (zip_uint64_t i = 0; i < (zip_uint64_t)numEntries; i++)
    {
        const char* name = zip_get_name(archive, i, 0);
        if (!name)
        {
            std::cerr << "Failed to get entry name for index " << i << '\n';
            zip_close(archive);
            return 1;
        }

        std::string outPath = extractDir + "/" + name;

        if (name[strlen(name) - 1] == '/')
        {
            std::filesystem::create_directories(outPath);
        }
        else
        {
            std::filesystem::create_directories(std::filesystem::path(outPath).parent_path());

            zip_file* zfile = zip_fopen_index(archive, i, 0);
            if (!zfile)
            {
                std::cerr << "Failed to open file inside zip: " << name << '\n';
                zip_close(archive);
                return 1;
            }

            FILE* outfile = fopen(outPath.c_str(), "wb");
            if (!outfile)
            {
                std::cerr << "Failed to create output file: " << outPath << '\n';
                zip_fclose(zfile);
                zip_close(archive);
                return 1;
            }

            char buffer[4096];
            zip_int64_t bytesRead = 0;
            while ((bytesRead = zip_fread(zfile, buffer, sizeof(buffer))) > 0)
            {
                fwrite(buffer, 1, bytesRead, outfile);
            }

            fclose(outfile);
            zip_fclose(zfile);
        }
    }

    zip_close(archive);
    std::cout << "Successfully extracted " << zipPath << " to " << extractDir << '\n';
    return 0;
}

void DownloadBlenderBenchmarks()
{
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    std::cout << "Downloading Blender benchmarks...\n";
    blenderBenchmarksDownloadStatus = "Downloading Blender benchmarks...";
    if (DownloadFile("https://opendata.blender.org/snapshots/opendata-latest.zip",
                     "tmp/opendata.zip") != 0)
    {
        std::cerr << "Failed to download Blender Open Data!\n";
        return;
    }

    if (!std::filesystem::exists("tmp/opendata"))
    {
        std::filesystem::create_directory("tmp/opendata");
    }
    std::cout << "Extracting Blender benchmarks...\n";
    blenderBenchmarksDownloadStatus = "Extracting Blender benchmarks...";
    if (UnzipFile("tmp/opendata.zip", "tmp/opendata") != 0)
    {
        std::cerr << "Failed to uzip Blender Open Data!\n";
        return;
    }

    std::filesystem::remove("tmp/opendata.zip");
}

void ProcessBlenderBenchmarkEntry(simdjson::dom::element entry,
                                  std::vector<BenchmarkEntry>& rawBenchmarks)
{
    auto deviceInfo = entry["device_info"];
    auto computeDevice = deviceInfo["compute_devices"].at(0);

    std::string deviceName =
        std::string(computeDevice->is_string() ? computeDevice : computeDevice["name"]);
    for (size_t i = 0; i < deviceName.size(); i++)
    {
        if (deviceName[i] == ',') deviceName.erase(deviceName.begin() + i);
    }

    DeviceType deviceType =
        (deviceInfo["device_type"].get_string().value() == "CPU" ? DeviceType::CPU
                                                                 : DeviceType::GPU);

    if (!entry["stats"].error())
    {
        // Skip entries that do not have the samples_per_minute field
        if (entry["stats"]["samples_per_minute"].error()) return;

        double score = entry["stats"]["samples_per_minute"].get_double();
        std::string sceneName = std::string(entry["scene"]["label"]);
        rawBenchmarks.emplace_back(deviceName, sceneName, deviceType, score);
    }
    else
    {
        // Skip old Blender data that does not have the samples_per_minute field
        return;
        for (auto scene: entry["scenes"])
        {
            if (scene["stats"]["result"]->get_string().value() == "CRASH") continue;
            double score = scene["stats"]["samples_per_minute"].get_double();
            std::string sceneName = std::string(scene["name"]);
            rawBenchmarks.emplace_back(deviceName, sceneName, deviceType, score);
        }
    }
}

std::unordered_map<std::string, double>
GetSceneCoefficients(const std::vector<BenchmarkEntry>& benchmarks)
{
    std::unordered_map<std::string, std::vector<double>> sceneScores;
    for (size_t i = 0; i < benchmarks.size(); i++)
    {
        sceneScores[benchmarks[i].scene].push_back(benchmarks[i].score);
    }
    for (auto& entry: sceneScores)
    {
        std::sort(entry.second.begin(), entry.second.end());
    }

    std::unordered_map<std::string, double> sceneCoefficients;
    for (auto& entry: sceneScores)
    {
        sceneCoefficients[entry.first] = entry.second[entry.second.size() / 2];
    }
    return sceneCoefficients;
}

std::unordered_map<std::string, BenchmarkEntry> ProcessBlenderBenchmarks()
{
    // Download if missing
    if (!std::filesystem::exists("tmp/opendata")) DownloadBlenderBenchmarks();

    // Find the benchmarks file
    std::string benchmarksPath = "";
    for (auto entry: std::filesystem::directory_iterator("tmp/opendata"))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".jsonl")
        {
            benchmarksPath = entry.path().string();
            break;
        }
    }
    if (benchmarksPath == "")
    {
        std::cerr << "Failed to find the benchmarks file!\n";
        return {};
    }
    std::cout << benchmarksPath << '\n';

    // Process the file
    std::cout << "Processing the Blender benchmarks file...\n";
    blenderBenchmarksDownloadStatus = "Processing Blender benchmarks...";
    simdjson::dom::parser parser;
    std::vector<BenchmarkEntry> rawBenchmarks;
    std::ifstream benchmarksFile(benchmarksPath);
    std::string line;
    while (std::getline(benchmarksFile, line))
    {
        simdjson::dom::element doc;
        auto error = parser.parse(line).get(doc);
        if (error)
        {
            std::cerr << error << "\n";
            continue;
        }

        // Extract data
        auto data = doc["data"];
        if (data.is_array())
        {
            for (auto entry: data)
            {
                ProcessBlenderBenchmarkEntry(entry, rawBenchmarks);
            }
        }
        else
        {
            ProcessBlenderBenchmarkEntry(data.value(), rawBenchmarks);
        }
    }
    std::sort(rawBenchmarks.begin(), rawBenchmarks.end());

    // Get scene coefficients
    auto sceneCoefficients = GetSceneCoefficients(rawBenchmarks);

    // Remove duplicates by finding the median average of the same devices' scores
    std::cout << "Removing duplicates...\n";
    blenderBenchmarksDownloadStatus = "Removing duplicates...";
    std::unordered_map<std::string, BenchmarkEntry> benchmarks;
    std::vector<double> scores;
    std::string lastName = rawBenchmarks[0].name;
    for (size_t i = 0; i < rawBenchmarks.size(); i++)
    {
        if (rawBenchmarks[i].name != lastName)
        {
            std::sort(scores.begin(), scores.end());
            auto& benchmark = rawBenchmarks[i];
            benchmarks[benchmark.name] = {benchmark.name, benchmark.type,
                                          scores[scores.size() / 2]};
            scores.clear();
        }
        scores.push_back(sceneCoefficients[rawBenchmarks[i].scene] * rawBenchmarks[i].score);
        lastName = rawBenchmarks[i].name;
    }

    // Cache processed data
    FilterBenchmarks(benchmarks);
    blenderBenchmarksDownloadStatus = "Finished processing Blender benchmarks!";
    benchmarksAvailable = true;
    return benchmarks;
}

std::unordered_map<std::string, BenchmarkEntry> GetBlenderBenchmarks()
{
    if (std::filesystem::exists(BENCHMARKS_PATH))
    {
        std::ifstream file(BENCHMARKS_PATH);
        std::string line, benchmarksString;
        while (std::getline(file, line))
        {
            benchmarksString += line;
        }
        auto benchmarksSplit = Split(benchmarksString);
        std::unordered_map<std::string, BenchmarkEntry> benchmarks;
        for (size_t i = 0; i < benchmarksSplit.size(); i += 3)
        {
            auto name = benchmarksSplit[i];
            auto type = (benchmarksSplit[i + 1] == "CPU" ? DeviceType::CPU : DeviceType::GPU);
            auto score = stod(benchmarksSplit[i + 2]);
            benchmarks[name] = {name, type, score};
        }
        return benchmarks;
    }
    else
        return ProcessBlenderBenchmarks();
}
