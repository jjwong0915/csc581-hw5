#include "event/timestamp.hpp"

void std::chrono::to_json(json& j, const timestamp& t) {
  j = t.time_since_epoch().count();
}

void std::chrono::from_json(const json& j, timestamp& t) {
  t = timestamp{std::chrono::duration<int64_t, std::nano>(j)};
}
