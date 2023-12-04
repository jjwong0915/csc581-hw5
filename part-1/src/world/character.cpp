#include "world/character.hpp"

void to_json(json& j, const character& c) {
  j = {
      {"x", c.x},
      {"y", c.y},
      {"move_left", c.move_left},
      {"move_right", c.move_right},
      {"velocity_y", c.velocity_y},
  };
}

void from_json(const json& j, character& c) {
  j.at("x").get_to(c.x);
  j.at("y").get_to(c.y);
  j.at("move_left").get_to(c.move_left);
  j.at("move_right").get_to(c.move_right);
  j.at("velocity_y").get_to(c.velocity_y);
}
