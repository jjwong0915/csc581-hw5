#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct obstacle {
  float x;
  float y;
  float w;
  float h;
};

void to_json(json& j, const obstacle& c);
void from_json(const json& j, obstacle& c);

#endif
