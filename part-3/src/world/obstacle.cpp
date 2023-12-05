#include "world/obstacle.hpp"

void to_json(json& j, const obstacle& c) {
  j = {
      {"x", c.x},
      {"y", c.y},
      {"w", c.w},
      {"h", c.h},
  };
}

void from_json(const json& j, obstacle& c) {
  j.at("x").get_to(c.x);
  j.at("y").get_to(c.y);
  j.at("w").get_to(c.w);
  j.at("h").get_to(c.h);
}
