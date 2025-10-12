#include "data_loader.hpp"

// implement the function that maps json data to a struct type
std::vector<Tick> json_to_tickDataVector(const json& jsonData) {
    std::vector<Tick> tickDataVector;
    const auto& data = jsonData.at("data");

    for (const auto& tickEntry : data) {
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