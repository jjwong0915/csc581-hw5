#include "event/timeline.hpp"

#include <chrono>

timestamp timeline::get_time() { return std::chrono::steady_clock::now(); }
