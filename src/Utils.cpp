// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#define CURL_STATICLIB
#include "Utils.hpp"
#include <curl/curl.h>

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
