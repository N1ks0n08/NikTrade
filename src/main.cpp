#include <fmt/core.h>
#include <cstdlib>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>

// create a stock ticker data type
struct Tick {
    std::string date;
    double open, high, low, close;
    int volume;
};

int main() {
    const char* token_cstr = std::getenv("STOCKDATA_API_TOKEN");
    if (!token_cstr) {
        fmt::print("Error: Token not found.\n");
        return 1;
    }

    // Convert to std::string
    std::string token(token_cstr);

    // Trim trailing whitespace/newlines
    token.erase(token.find_last_not_of(" \n\r\t") + 1);

    // build and send a request to StockData from April 2025 to June 2025 (End Of Day Historical Data)
    
    std::string request = fmt::format("https://api.stockdata.org/v1/data/eod?symbols=AAPL&api_token={}&interval=day&sort=asc&date_from=2025-01-01", token);
    fmt::print("Request url: {}\n", request);
    cpr::Response apiResponse = cpr::Get(cpr::Url{request}, cpr::VerifySsl(false));

    if (apiResponse.status_code != 200) {
        fmt::print("Error: Failed to fetch data! Status code: {}", apiResponse.status_code);
        fmt::print("Status: {}\nError: {}\nBody: {}\n", apiResponse.status_code, apiResponse.error.message, apiResponse.text);
        return 1;
    }

    std::string filename = "AAPL_2025.json";
    
    // Create and write the API resposne to a file
    std::ofstream out(filename);
    if (!out) {
        fmt::print("Error: Could not create file!");
        return 1;
    }

    out << apiResponse.text;
    out.close();

    fmt::print("JSON saved to {}", filename);

    return 0;
}
