#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct platform {
  float x;
  float y;
};

void to_json(json& j, const platform& c);
void from_json(const json& j, platform& c);

#endif
