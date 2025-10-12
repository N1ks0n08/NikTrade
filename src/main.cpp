#include <fmt/core.h>
#include <cstdlib>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <fstream>

/*
SAMPLE JSON DATA FORMAT FROM StockData.org End Of Day DATA API:
{
    "meta": {
        "date_from": "2023-03-18",
        "date_to": "2023-09-14",
        "max_period_days": 180
    },
    "data": [
        {
            "date": "2023-09-12T00:00:00.000Z",
            "open": 179.49,
            "high": 180.11,
            "low": 174.84,
            "close": 176.29,
            "volume": 1454605
        }, ...
    ]
}
*/

// Make a json alias
using json = nlohmann::json;

// create a stock ticker data type
struct Tick {
    std::string date;
    double open, high, low, close;
    int volume;
};

// create a function that maps json data to a struct type
std::vector<Tick> json_to_tickDataVector(json& jsonData) {
    std::vector<Tick> tickDataVector;
    const auto& data = jsonData.at("data");

    for (int index = 0; index < data.size(); index++) {
        const auto& tickEntry = data[index];
        Tick tickdata;
        tickEntry.at("date").get_to(tickdata.date);
        tickEntry.at("open").get_to(tickdata.open);
        tickEntry.at("high").get_to(tickdata.high);
        tickEntry.at("low").get_to(tickdata.low);
        tickEntry.at("close").get_to(tickdata.close);
        tickEntry.at("volume").get_to(tickdata.volume);

        tickDataVector.push_back(tickdata);
    }

    return tickDataVector;
}

void printData(std::vector<Tick>& tickDataVector) {
    for (int index = 0; index < tickDataVector.size(); index++) {
        fmt::print("The highest SPY price today on {} was: {}\n", tickDataVector[index].date, tickDataVector[index].high;
    }
}

int main() {
    // Open and load a JSON file pertaining to SPY market data
    std::ifstream file ("SPY_2025.json");
    if (!file.is_open()) {
        fmt::print("Error: failed to open file!\n");
        return 1;
    }

    // create an empty JSON object of nlohmann::json type
    json jsonData;
    // parse text from file into JSON structured object
    file >> jsonData;

    std::vector<Tick> tickDataVector = json_to_tickDataVector(jsonData);

    printData(tickDataVector);

    return 0;
}
