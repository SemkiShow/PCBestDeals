#define CURL_STATICLIB
#include "Prices.hpp"
#include <cmath>
#include <curl/curl.h>
#include <fstream>
#include <simdjson.h>
#include <vector>

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

        std::string postFields = "grant_type=client_credentials&scope=";
        postFields += sandbox ? "https://api.ebay.com/oauth/api_scope/buy.item.feed"
                              : "https://api.ebay.com/oauth/api_scope/buy.browse";

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
