#ifndef WORLD_HPP
#define WORLD_HPP

#include <nlohmann/json.hpp>
#include <vector>

#include "event/timestamp.hpp"
#include "world/character.hpp"
#include "world/finish.hpp"
#include "world/platform.hpp"

using json = nlohmann::json;

struct world {
  timestamp time;
  character main_character;
  std::vector<platform> platforms;
  platform moving_platform;
  finish finish_point;
  bool finished;
};

void to_json(json& j, const world& w);
void from_json(const json& j, world& w);

#endif