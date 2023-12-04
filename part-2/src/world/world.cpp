#include "world/world.hpp"

#include <iostream>

void to_json(json& j, const world& w) {
  j = {
      {"time", w.time},
      {"main_character", w.main_character},
      {"platforms", w.platforms},
      {"moving_platform", w.moving_platform},
      {"finish_point", w.finish_point},
      {"finished", w.finished},
  };
}

void from_json(const json& j, world& w) {
  j.at("time").get_to(w.time);
  j.at("main_character").get_to(w.main_character);
  j.at("platforms").get_to(w.platforms);
  j.at("moving_platform").get_to(w.moving_platform);
  j.at("finish_point").get_to(w.finish_point);
  j.at("finished").get_to(w.finished);
}
