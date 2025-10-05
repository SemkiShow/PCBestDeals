// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#define CURL_STATICLIB
#include "Prices.hpp"
#include "Settings.hpp"
#include "Utils.hpp"
#include <cmath>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <simdjson.h>
#include <vector>

std::string ebayPricesDownloadStatus = "";

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* s)
{
    size_t newLength = size * nmemb;
    s->append((char*)contents, newLength);
    return newLength;
}

std::string Base64Encode(const std::string& input)
{
    static const char* base64Chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string output;
    int val = 0, valb = -6;
    for (unsigned char c: input)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            output.push_back(base64Chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) output.push_back(base64Chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (output.size() % 4)
        output.push_back('=');
    return output;
}

std::array<std::string, 2> GetANewEbayToken(const std::string& clientID,
                                            const std::string& clientSecret, bool sandbox = false)
{
    // Get the token
    std::cout << "Getting a new ebay token...\n";
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string authPlain = clientID + ":" + clientSecret;
    std::string authHeader = "Authorization: Basic " + Base64Encode(authPlain);

    std::string url = sandbox ? "https://api.sandbox.ebay.com/identity/v1/oauth2/token"
                              : "https://api.ebay.com/identity/v1/oauth2/token";

    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
        headers = curl_slist_append(headers, authHeader.c_str());

        std::string postFields =
            "grant_type=client_credentials&scope=https://api.ebay.com/oauth/api_scope";

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);

        if (res != CURLE_OK)
        {
            std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
            return {"", ""};
        }
    }

    // Parse JSON response
    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    auto error = parser.parse(readBuffer).get(doc);
    if (error)
    {
        std::cerr << "Parse error: " << error << "\n";
        return {"", ""};
    }

    std::string_view token;
    if (doc["access_token"].get(token) == simdjson::SUCCESS)
    {
        std::string expiresIn = std::to_string(doc["expires_in"]->get_int64());
        return {std::string(token), expiresIn};
    }
    else
    {
        std::cerr << "access_token not found. Response: " << readBuffer << "\n";
        return {"", ""};
    }
}

std::string GetEbayToken(bool sandbox)
{
    // Load credentials
    std::fstream credentialsFile("credentials.txt", std::ios::in);
    std::string buf;
    std::vector<std::string> credentials;
    while (std::getline(credentialsFile, buf))
    {
        credentials.push_back(buf);
    }
    credentialsFile.close();

    if (credentials.size() < 4 || stoi(credentials[3]) < time(0))
    {
        credentials.resize(std::fmax(4, credentials.size()));
        auto token = GetANewEbayToken(credentials[0], credentials[1], sandbox);
        credentials[2] = token[0];
        credentials[3] = std::to_string(time(0) + stol(token[1]));

        credentialsFile.open("credentials.txt", std::ios::out);
        for (size_t i = 0; i < credentials.size(); i++)
        {
            credentialsFile << credentials[i] << '\n';
        }
        credentialsFile.close();
    }

    return credentials[2];
}

std::string UrlEncode(const std::string& input)
{
    std::ostringstream output;
    output.fill('0');
    output << std::hex;

    for (const auto& c: input)
    {
        // Unreserved characters according to RFC 3986
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            output << c;
        }
        else
        {
            output << '%' << std::uppercase << std::setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return output.str();
}

std::vector<DealEntry> GetEbayDeals(const std::string& query, const std::string& token,
                                    bool recursive, bool sandbox, int offset)
{
    if (query == "" || find(query.begin(), query.end(), '*') != query.end()) return {};
    CURL* curl = curl_easy_init();
    std::string readBuffer;
    int limit = 200;

    if (curl)
    {
        std::string url = "https://api.";
        if (sandbox) url += "sandbox.";
        url += "ebay.com/buy/browse/v1/item_summary/search?q=" + UrlEncode(query) +
               "&limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset);
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + token).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";

        curl_easy_cleanup(curl);
    }

    // Parse JSON
    simdjson::dom::parser parser;
    simdjson::dom::element doc;
    auto error = parser.parse(readBuffer).get(doc);
    if (error)
    {
        std::cerr << "JSON parse error: " << error << "\n";
        return {};
    }

    if (!doc["errors"].error())
    {
        std::cout << readBuffer << '\n';
        exit(1);
    }
    if (!doc["total"].error() && doc["total"]->get_int64() == 0) return {};

    std::vector<DealEntry> deals;
    for (auto item: doc["itemSummaries"])
    {
        std::string title, price;
        if (!item["title"].error())
        {
            title = std::string(item["title"]);
        }
        if (!item["price"].error() && !item["price"]["value"].error())
        {
            price = std::string(item["price"]["value"]);
        }

        // std::cout << title << " - " << price << "\n";
        deals.emplace_back(title, stod(price));
    }

    std::cout << "\rProcessed " << std::fmin(offset + limit, doc["total"]->get_int64().value())
              << " items of " << doc["total"]->get_int64() << std::flush;

    if (offset + limit < doc["total"]->get_int64() && recursive)
    {
        // Process deals further than the first page
        // (can be used only once a day because of the rate limit)
        auto moreDeals = GetEbayDeals(query, token, recursive, sandbox, offset + limit);
        for (auto deal: moreDeals)
        {
            deals.push_back(deal);
        }
    }
    else
    {
        std::cout << '\n';
    }

    return deals;
}

bool IsPricesDownloadComplete(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks,
                              std::unordered_map<std::string, DealEntry>& prices)
{
    std::string partPricesString;
    std::ifstream file(PRICES_PATH);
    std::string buf;
    while (std::getline(file, buf))
    {
        partPricesString += buf;
    }
    file.close();
    auto rawPartPrices = Split(partPricesString);
    for (size_t i = 0; i + 1 < rawPartPrices.size(); i += 2)
    {
        prices[rawPartPrices[i]] = {rawPartPrices[i], stod(rawPartPrices[i + 1])};
    }
    for (auto& benchmark: benchmarks)
    {
        if (prices.find(benchmark.first) == prices.end()) return false;
    }
    return true;
}

std::unordered_map<std::string, DealEntry>
DownloadEbayPartPrices(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks,
                       const std::string& token, bool recursive, bool sandbox)
{
    std::unordered_map<std::string, DealEntry> prices;
    bool isPricesDownloadComplete = IsPricesDownloadComplete(benchmarks, prices);
    std::ofstream file(PRICES_PATH);

    if (!isPricesDownloadComplete)
    {
        // The previous download is incomplete, preserve data
        std::cout << prices.size() << '\n';
        for (auto& price: prices)
        {
            file << price.second.name << ',' << price.second.price;
            file << ",\n";
        }
        file << std::flush;
    }
    else
    {
        // The previous download is complete, start from scratch
        prices.clear();
    }

    size_t index = 0;
    for (auto& benchmark: benchmarks)
    {
        if (prices.find(benchmark.first) != prices.end())
        {
            index++;
            continue;
        }
        std::cout << "Getting the price of item " << index + 1 << " of " << benchmarks.size()
                  << " (" << benchmark.second.name << ")\n";
        ebayPricesDownloadStatus = "Getting the price of item " + std::to_string(index + 1) +
                                   " of " + std::to_string(benchmarks.size()) + "\n(" +
                                   benchmark.second.name + ")";
        auto deals = GetEbayDeals(benchmark.second.name, token, recursive, sandbox);
        std::sort(deals.begin(), deals.end());
        if (deals.size() == 0) deals.emplace_back(benchmark.second.name, -1);

        prices[benchmark.first] = {benchmark.second.name, deals[deals.size() / 2].price};
        file << prices[benchmark.first].name << ',' << prices[benchmark.first].price;
        if (index++ < benchmarks.size() - 1) file << ",\n";
        file << std::flush;
    }
    file.close();
    FilterPrices(prices);
    ebayPricesDownloadStatus = "Finished downloading ebay prices!";
    pricesAvailable = true;
    return prices;
}

std::unordered_map<std::string, DealEntry>
GetEbayPartPrices(const std::unordered_map<std::string, BenchmarkEntry>& benchmarks,
                  const std::string& token, bool sandbox)
{
    if (std::filesystem::exists(PRICES_PATH))
    {
        std::string partPricesString;
        std::ifstream file(PRICES_PATH);
        std::string buf;
        while (std::getline(file, buf))
        {
            partPricesString += buf;
        }
        file.close();
        auto rawPartPrices = Split(partPricesString);
        std::unordered_map<std::string, DealEntry> prices;
        for (size_t i = 0; i + 1 < rawPartPrices.size(); i += 2)
        {
            prices[rawPartPrices[i]] = {rawPartPrices[i], stod(rawPartPrices[i + 1])};
        }
        return prices;
    }
    else
    {
        return DownloadEbayPartPrices(benchmarks, token, sandbox);
    }
}
