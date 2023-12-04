#include "world/finish.hpp"

void to_json(json& j, const finish& c) {
  j = {
      {"x", c.x},
      {"y", c.y},
  };
}

void from_json(const json& j, finish& c) {
  j.at("x").get_to(c.x);
  j.at("y").get_to(c.y);
}
