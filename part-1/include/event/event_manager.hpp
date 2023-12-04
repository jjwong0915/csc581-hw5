#ifndef EVENT_MANAGER_HPP
#define EVENT_MANAGER_HPP

#include <chrono>
#include <functional>
#include <map>
#include <queue>
#include <string>
#include <vector>

#include "event/event.hpp"
#include "event/timestamp.hpp"
#include "world/world.hpp"

using event_handler = std::function<void(world&, const event&)>;

class event_manager {
  std::map<std::string, std::vector<event_handler>> handlers;
  std::vector<event> raised_events;

 public:
  void register_handler(std::string event_type, event_handler handler);
  void raise_event(event e);
  void apply_events(world& w, timestamp max_time);
};

#endif
