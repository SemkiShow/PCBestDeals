#include "Benchmarks.hpp"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#define CURL_STATICLIB
#include <curl/curl.h>
#include <zip.h>

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
            std::cout << "Successfully extracted " << zipPath << " to " << extractDir << '\n';

            fclose(outfile);
            zip_fclose(zfile);
        }
    }

    zip_close(archive);
    return 0;
}

void DownloadBlenderBenchmarks()
{
    if (!std::filesystem::exists("tmp"))
    {
        std::filesystem::create_directory("tmp");
    }
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
    if (UnzipFile("tmp/opendata.zip", "tmp/opendata") != 0)
    {
        std::cerr << "Failed to uzip Blender Open Data!\n";
        return;
    }

    std::filesystem::remove("tmp/opendata.zip");
}
