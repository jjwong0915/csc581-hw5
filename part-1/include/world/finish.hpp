#ifndef FINISH_HPP
#define FINISH_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct finish {
  float x;
  float y;
};

void to_json(json& j, const finish& c);
void from_json(const json& j, finish& c);

#endif
