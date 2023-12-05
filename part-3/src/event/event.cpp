#include "event/event.hpp"

#include <chrono>
#include <iostream>

#include "event/timeline.hpp"

event::event() {}

event::event(
    std::string type,
    std::map<std::string, std::variant<std::string, double, bool>> param)
    : type{type}, param{param} {}

bool operator<(const event& e1, const event& e2) { return e1.time < e2.time; }

void to_json(json& j, const event& e) {
  j = {
      {"time", e.time},
      {"type", e.type},
      {"param", json::object()},
  };
  for (const auto& p : e.param) {
    std::visit([&](auto&& arg) { j.at("param")[p.first] = arg; }, p.second);
  }
}

void from_json(const json& j, event& e) {
  j.at("time").get_to(e.time);
  j.at("type").get_to(e.type);
  for (const auto& [key, value] : j.at("param").items()) {
    if (value.is_string()) {
      e.param[key] = value.template get<std::string>();
    } else if (value.is_number()) {
      e.param[key] = value.template get<double>();
    } else if (value.is_boolean()) {
      e.param[key] = value.template get<bool>();
    }
  }
}
