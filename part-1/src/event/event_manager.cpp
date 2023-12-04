#include "event/event_manager.hpp"

#include <algorithm>
#include <iostream>

#include "event/timeline.hpp"

void event_manager::register_handler(std::string event_type,
                                     event_handler handler) {
  handlers[event_type].push_back(handler);
}

void event_manager::raise_event(event e) {
  e.time = timeline::get_time();
  raised_events.push_back(e);
}

void event_manager::apply_events(world& w, timestamp max_time) {
  event marker_event;
  marker_event.time = w.time;
  auto next_event = std::upper_bound(raised_events.begin(), raised_events.end(),
                                     marker_event);
  while (next_event != raised_events.end() && next_event->time <= max_time) {
    for (auto handler : handlers[next_event->type]) {
      handler(w, *next_event);
    }
    w.time = next_event->time;
    next_event = std::next(next_event);
  }
}
