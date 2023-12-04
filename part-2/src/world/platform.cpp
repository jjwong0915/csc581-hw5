#include "world/platform.hpp"

void to_json(json& j, const platform& c) {
  j = {
      {"x", c.x},
      {"y", c.y},
  };
}

void from_json(const json& j, platform& c) {
  j.at("x").get_to(c.x);
  j.at("y").get_to(c.y);
}
