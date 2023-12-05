#ifndef TIMESTAMP_HPP
#define TIMESTAMP_HPP

#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using timestamp = std::chrono::time_point<std::chrono::steady_clock>;

namespace std::chrono {

void to_json(json& j, const timestamp& t);
void from_json(const json& j, timestamp& t);

}  // namespace std::chrono

#endif