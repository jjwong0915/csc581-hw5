#ifndef EVENT_HPP
#define EVENT_HPP

#include <chrono>
#include <map>
#include <nlohmann/json.hpp>
#include <string>
#include <variant>

#include "event/timestamp.hpp"

using json = nlohmann::json;

struct event {
  timestamp time;
  std::string type;
  std::map<std::string, std::variant<std::string, double, bool>> param;
  event();
  event(std::string type,
        std::map<std::string, std::variant<std::string, double, bool>> param);
};

bool operator<(const event& e1, const event& e2);
void to_json(json& j, const event& e);
void from_json(const json& j, event& e);

#endif