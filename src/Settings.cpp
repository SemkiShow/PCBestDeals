#include "Benchmarks.hpp"
#include "Prices.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

std::string lastCAUpdate = "";
bool benchmarksAvailable = false;
bool pricesAvailable = false;

void UpdateCACertificate()
{
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
    DownloadFile("https://curl.se/ca/cacert.pem", "tmp/cacert.pem");
    if (std::filesystem::exists("resources/cacert.pem"))
    {
        std::filesystem::remove("resources/cacert.pem");
    }
    std::filesystem::copy_file("tmp/cacert.pem", "resources/cacert.pem",
                               std::filesystem::copy_options::overwrite_existing);
    std::filesystem::remove("tmp/cacert.pem");
    std::cout << "Updated the CA certificate\n";
}

void CheckForDataAvailable()
{
    benchmarksAvailable = std::filesystem::exists(BENCHMARKS_PATH);
    if (benchmarksAvailable)
    {
        auto benchmarks = GetBlenderBenchmarks();
        std::vector<DealEntry> partPrices;
        pricesAvailable = IsPricesDownloadComplete(benchmarks, partPrices);
    }
    else
    {
        pricesAvailable = false;
    }
}

void Save()
{
    std::ofstream file("settings.txt");
    file << "last-ca-update=" << lastCAUpdate << '\n';
    file.close();
}

void Load()
{
    std::ifstream file("settings.txt");
    std::string buf, label, value;
    while (std::getline(file, buf))
    {
        auto split = Split(buf, '=');
        if (split.size() < 2)
        {
            std::cout << "Error: invalid settings.txt!\n";
            continue;
        }
        label = TrimJunk(split[0]);
        value = TrimJunk(split[1]);

        if (label == "last-ca-update") lastCAUpdate = value;
    }
    file.close();

    // Update the CA certificate, if necessary (done monthly)
    if (lastCAUpdate == "")
    {
        time_t now = time(0);
        lastCAUpdate = asctime(localtime(&now));
        UpdateCACertificate();
    }
    else
    {
        struct tm parsedTm;
        std::istringstream ss(lastCAUpdate);
        ss >> std::get_time(&parsedTm, "%a %b %d %H:%M:%S %Y");
        time_t reconstructedTime = mktime(&parsedTm);
        if (difftime(time(0), reconstructedTime) > 60 * 60 * 24 * 30)
        {
            UpdateCACertificate();
        }
    }

    CheckForDataAvailable();
}
