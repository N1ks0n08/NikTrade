#pragma once
#include <vector>
#include "tick.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;
// Declare function for json to vector<Tick> conversion
std::vector<Tick> json_to_tickDataVector(const json& jsonData);