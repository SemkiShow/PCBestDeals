// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#define CURL_STATICLIB
#include "Utils.hpp"
#include <algorithm>
#include <curl/curl.h>
#include <fstream>
#include <iostream>

const std::vector<std::string> badNames = {"AMD 100-000000011-11",
                                           "AMD EPYC",
                                           "AMD EPYC Processor",
                                           "AMD EPYC Processor (with IBPB)",
                                           "AMD EPYC-Genoa Processor",
                                           "AMD EPYC-Milan Processor",
                                           "AMD EPYC-Rome Processor",
                                           "AMD EPYC-Rome-v4 Processor (no XSAVES)",
                                           "AMD Eng Sample ZS288057TGG54_35/28/20_2/16",
                                           "AMD Eng Sample: 100-000000053-04_32/20_N",
                                           "AMD Eng Sample: 100-000000054-04_32/24_N",
                                           "AMD Eng Sample: 100-000000163-01_43/29_Y",
                                           "AMD Eng Sample: 100-000000425_37/24_N",
                                           "AMD Eng Sample: 100-000000475-15",
                                           "AMD Eng Sample: 100-000000475-20",
                                           "AMD Eng Sample: 100-000000514-20_Y",
                                           "AMD Eng Sample: 100-000000560-40_Y",
                                           "AMD Eng Sample: 100-000000870-32_Y",
                                           "AMD Eng Sample: 100-000000894-04",
                                           "AMD Eng Sample: 100-000000896-02",
                                           "AMD Eng Sample: 100-000000897-03",
                                           "AMD Eng Sample: 100-000000954-50_Y",
                                           "AMD Eng Sample: 100-000000994-38_Y",
                                           "AMD Eng Sample: 100-000000997-01",
                                           "AMD Eng Sample: 100-000001243-50_Y",
                                           "AMD Eng Sample: 100-000001247-12",
                                           "AMD Eng Sample: 100-000001277-60_Y",
                                           "AMD Eng Sample: 100-000001535-05",
                                           "AMD Eng Sample: 2D3151A2M88E4_35/31_N",
                                           "AMD Eng Sample: ZS1406E2VJUG5_22/14_N",
                                           "Apple Silicon",
                                           "Apple processor",
                                           "Apple silicon",
                                           "Common KVM processor",
                                           "DG1002FGF84HT",
                                           "Genuine Intel 0000",
                                           "Genuine Intel 0000 2.40GHz",
                                           "Genuine Intel 0000 @",
                                           "Genuine Intel 0000 @ 1.80GHz",
                                           "Genuine Intel CPU 0000 @ 1.60GHz",
                                           "Genuine Intel CPU 0000 @ 1.80GHz",
                                           "Genuine Intel CPU 0000 @ 2.00GHz",
                                           "Genuine Intel CPU 0000 @ 2.10GHz",
                                           "Genuine Intel CPU 0000 @ 2.20GHz",
                                           "Genuine Intel CPU 0000 @ 2.40GHz",
                                           "Genuine Intel CPU 0000 @ 2.50GHz",
                                           "Genuine Intel CPU 0000 @ 2.60GHz",
                                           "Genuine Intel CPU 0000 @ 2.80GHz",
                                           "Genuine Intel CPU 0000 @ 3.00GHz",
                                           "Genuine Intel CPU 0000 @ 3.10GHz",
                                           "Genuine Intel CPU 0000 @ 3.30GHz",
                                           "Genuine Intel CPU 0000%@",
                                           "Genuine Intel CPU @ 2.20GHz",
                                           "Genuine Intel CPU @ 2.30GHz",
                                           "Genuine Intel CPU @ 2.40GHz",
                                           "Genuine Intel CPU @ 2.80GHz",
                                           "Genuine Intel(R) CPU  @ 2.50GHz",
                                           "Genuine Intel(R) CPU 0000 @ 2.10GHz",
                                           "Genuine Intel(R) CPU 0000 @ 2.20GHz",
                                           "Genuine Intel(R) CPU 0000 @ 2.50GHz",
                                           "High Performance Datacenter vCPU",
                                           "Intel Core 2 Duo P9xxx (Penryn Class Core 2)",
                                           "Intel Core Processor (Broadwell IBRS)",
                                           "Intel Core Processor (Broadwell no TSX IBRS)",
                                           "Intel Core Processor (Haswell no TSX)",
                                           "Intel Core Processor (Skylake IBRS)",
                                           "Intel Xeon CPU @ 2.00GHz",
                                           "Intel Xeon CPU @ 2.20GHz",
                                           "Intel Xeon CPU @ 2.60GHz",
                                           "Intel Xeon CPU @ 2.80GHz",
                                           "Intel Xeon E312xx (Sandy Bridge)",
                                           "Intel Xeon Platinum",
                                           "Intel Xeon Processor (Cascadelake)",
                                           "Intel Xeon Processor (Icelake)",
                                           "Intel Xeon Processor (SapphireRapids)",
                                           "Intel Xeon Processor (Skylake IBRS)",
                                           "QEMU Virtual CPU version 2.5+",
                                           "Virtual CPU @ 1.00GHz",
                                           "Virtual CPU @ 2.00GHz",
                                           "Virtual CPU @ 2.41GHz",
                                           "Virtual CPU @ 2.55GHz",
                                           "Virtual CPU @ 2.95GHz",
                                           "Virtual CPU @ 2.99GHz",
                                           "Virtual CPU @ 3.20GHz",
                                           "Virtual CPU @ 3.24GHz",
                                           "Virtual CPU @ 3.41GHz",
                                           "Westmere E56xx/L56xx/X56xx (Nehalem-C)",
                                           "ZHAOXIN KaiXian KX-U6780A@2.7GHz",
                                           "",
                                           "6869:00 [ZLUDA]",
                                           "AMD Custom GPU 0405",
                                           "AMD Radeon (TM) Graphics",
                                           "AMD Radeon (TM) Pro VII",
                                           "AMD Radeon Graphics",
                                           "AMD Radeon PRO Graphics",
                                           "AMD Radeon Polaris",
                                           "AMD Radeon Pro GFX10 Family Unknown Prototype",
                                           "AMD Radeon RX Ellesmere Prototype",
                                           "AMD Radeon TM Graphics",
                                           "AMD Radeon VII",
                                           "AMD Radeon VII Series",
                                           "AMD Radeon VII Series [ZLUDA]",
                                           "AMD Radeon Vega",
                                           "AMD Radeon Vega Frontier Edition",
                                           "AMD Radeon(TM) Graphics",
                                           "AMD Radeon(TM) Graphics [ZLUDA]",
                                           "AMD Radeon(TM) Pro Graphics",
                                           "EIZO Quadro MED-XN92",
                                           "Intel(R) Graphics",
                                           "Intel(R) Graphics [0x5690]",
                                           "Intel(R) Graphics [0x56a0]",
                                           "Intel(R) Graphics [0x56a1]",
                                           "Intel(R) Graphics [0x56a5]",
                                           "Intel(R) Graphics [0xe20b]",
                                           "NVIDIA Graphics Device",
                                           "Navi 23 [Radeon RX 6600/6600 XT/6600M]",
                                           "P104-100",
                                           "P106-090",
                                           "P106-100",
                                           "PCI\\VEN_1002&DEV_164E&SUBSYS_D0001458&REV_C2",
                                           "Radeon 5",
                                           "Radeon 7",
                                           "Ryzen 5",
                                           "Unknown AMD GPU"};

std::vector<std::string> Split(const std::string& input, const char delimiter)
{
    std::vector<std::string> output(1, "");
    int index = 0;
    for (size_t i = 0; i < input.size(); i++)
    {
        if (input[i] == delimiter)
        {
            index++;
            output.push_back("");
            continue;
        }
        output[index] += input[i];
    }
    if (output.size() == 1 && output[0] == "") output.clear();
    return output;
}

std::string TrimJunk(const std::string& input)
{
    auto first = input.find_first_not_of("\t\n\r\f\v");
    auto last = input.find_last_not_of("\t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last - first + 1);
}

size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

int DownloadFile(const std::string& url, const std::string& outputPath)
{
    FILE* file = fopen(outputPath.c_str(), "wb");
    if (!file) return 1;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::cerr << "Failed to initialize CURL\n";
        fclose(file);
        return 1;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "PCBestDeals");
    curl_easy_setopt(curl, CURLOPT_CAINFO, "resources/cacert.pem");
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    return (res == CURLE_OK) ? 0 : 1;
}

void WriteBenchmarks(const std::vector<BenchmarkEntry>& benchmarks)
{
    std::ofstream file(BENCHMARKS_PATH);
    for (size_t i = 0; i < benchmarks.size(); i++)
    {
        file << benchmarks[i].name << ',' << benchmarks[i].type << ',' << benchmarks[i].score;
        if (i < benchmarks.size() - 1) file << ",\n";
    }
    file.close();
}

void WritePrices(const std::vector<DealEntry>& prices)
{
    std::ofstream file(PRICES_PATH);
    for (size_t i = 0; i < prices.size(); i++)
    {
        file << prices[i].name << ',' << prices[i].price;
        if (i < prices.size() - 1) file << ",\n";
    }
    file.close();
}

void FilterBenchmarks(std::vector<BenchmarkEntry>& benchmarks)
{
    for (size_t i = 0; i < benchmarks.size(); i++)
    {
        if (std::find(badNames.begin(), badNames.end(), benchmarks[i].name) != badNames.end())
        {
            benchmarks.erase(benchmarks.begin() + i);
            i--;
        }
    }
    WriteBenchmarks(benchmarks);
}

void FilterData(std::vector<BenchmarkEntry>& benchmarks, std::vector<DealEntry>& prices)
{
    if (benchmarks.size() != prices.size())
    {
        std::cerr << "The benchmarks do not match the prices!\n";
        return;
    }
    for (size_t i = 0; i < benchmarks.size(); i++)
    {
        if (std::find(badNames.begin(), badNames.end(), benchmarks[i].name) != badNames.end())
        {
            benchmarks.erase(benchmarks.begin() + i);
            prices.erase(prices.begin() + i);
            i--;
        }
    }
    WriteBenchmarks(benchmarks);
    WritePrices(prices);
}
