#ifndef WORLD_HPP
#define WORLD_HPP

#include <deque>
#include <nlohmann/json.hpp>

#include "event/timestamp.hpp"
#include "world/character.hpp"
#include "world/obstacle.hpp"
#include "world/platform.hpp"

using json = nlohmann::json;

struct world {
  timestamp time;
  character main_character;
  std::deque<platform> platforms;
  std::deque<obstacle> obstacles;
  int score;
  bool game_over;
  bool game_start;
};

void to_json(json& j, const world& w);
void from_json(const json& j, world& w);

#endif