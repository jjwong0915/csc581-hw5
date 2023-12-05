#include "world/world.hpp"

#include <iostream>

void to_json(json& j, const world& w) {
  j = {
      {"time", w.time},
      {"main_character", w.main_character},
      {"platforms", w.platforms},
      {"obstacles", w.obstacles},
      {"score", w.score},
      {"game_over", w.game_over},
      {"game_start", w.game_start},
  };
}

void from_json(const json& j, world& w) {
  j.at("time").get_to(w.time);
  j.at("main_character").get_to(w.main_character);
  j.at("platforms").get_to(w.platforms);
  j.at("obstacles").get_to(w.obstacles);
  j.at("score").get_to(w.score);
  j.at("game_over").get_to(w.game_over);
  j.at("game_start").get_to(w.game_start);
}
