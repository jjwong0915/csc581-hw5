#ifndef CHARACTER_HPP
#define CHARACTER_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct character {
  float x;
  float y;
  bool move_right;
  bool move_left;
  float velocity_y;
};

void to_json(json& j, const character& c);
void from_json(const json& j, character& c);

#endif