#include <v8/libplatform/libplatform.h>
#include <v8/v8.h>

#include <SFML/Window.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
#include <random>
#include <thread>
#include <variant>
#include <zmq.hpp>

#include "event/event_manager.hpp"
#include "event/timeline.hpp"
#include "script_manager/script_manager.hpp"
#include "world/world.hpp"

using json = nlohmann::json;
using namespace std::chrono;

std::mutex global_mtx;
event_manager global_event;
world global_world{
    timeline::get_time(), {290, 600, false, false, 0}, {{250, 700}}, 0, false,
};
script_manager global_script{global_event, global_world};

// convenience reference for main character
auto& main_char = global_world.main_character;

// random number generator
std::random_device rd;
std::default_random_engine re(rd());

void receive_event() {
  // setup socket
  zmq::context_t context;
  zmq::socket_t event_socket{context, ZMQ_REP};
  event_socket.bind("tcp://127.0.0.1:5555");
  // handle request
  while (true) {
    zmq::message_t event_req;
    if (event_socket.recv(event_req)) {
      std::lock_guard lock{global_mtx};
      global_event.raise_event(json::parse(event_req.to_string()));
    }
    event_socket.send(zmq::message_t{}, zmq::send_flags::none);
  }
}

void send_world() {
  // setup socket
  zmq::context_t context;
  zmq::socket_t world_socket{context, ZMQ_REP};
  world_socket.bind("tcp://127.0.0.1:5556");
  // handle request
  while (true) {
    zmq::message_t world_req;
    if (world_socket.recv(world_req)) {
      // send world
      std::lock_guard lock{global_mtx};
      std::string world_json = json(global_world).dump();
      world_socket.send(zmq::message_t{world_json.begin(), world_json.end()},
                        zmq::send_flags::none);
    }
  }
}

void handle_character_movement(world& w, const event& e) {
  main_char.x = std::get<double>(e.param.at("x"));
  main_char.y = std::get<double>(e.param.at("y"));
}

void handle_character_collision(world& w, const event& e) {
  main_char.velocity_y = std::min(main_char.velocity_y, 0.0f);
  main_char.y =
      std::min(main_char.y,
               (float)std::get<double>(e.param.at("collision_level")) - 20.0f);
}

void handle_character_gravity(world& w, const event& e) {
  main_char.velocity_y +=
      500.0 * std::get<double>(e.param.at("elapsed_seconds"));
}

void handle_character_death(world& w, const event& e) { w.game_over = true; }

void handle_key_press(world& w, const event& e) {
  sf::Keyboard::Key key =
      (sf::Keyboard::Key)std::get<double>(e.param.at("key"));
  if (key == sf::Keyboard::Left) {
    main_char.move_left = true;
  } else if (key == sf::Keyboard::Right) {
    main_char.move_right = true;
  } else if (key == sf::Keyboard::Space) {
    if (main_char.velocity_y == 0) {
      main_char.velocity_y -= 250.0;
    }
  }
}

void handle_key_release(world& w, const event& e) {
  sf::Keyboard::Key key =
      (sf::Keyboard::Key)std::get<double>(e.param.at("key"));
  if (key == sf::Keyboard::Left) {
    main_char.move_left = false;
  } else if (key == sf::Keyboard::Right) {
    main_char.move_right = false;
  }
}

void handle_special_jump(world& w, const event& e) {
  // handle special_jump event using script
  global_script.run_script("./src/script/handle_special_jump.js");
}

void handle_platform_movement(world& w, const event& e) {
  for (auto& p : w.platforms) {
    p.y -= 100.0 * std::get<double>(e.param.at("elapsed_seconds"));
  }
  // remove platform go over top
  while (w.platforms.size() > 0 && w.platforms.front().y < 0) {
    w.platforms.pop_front();
  }
  // add plaform from bottom
  if (w.platforms.empty() || w.platforms.back().y < 700) {
    float x = std::uniform_real_distribution<float>(0, 500)(re);
    w.platforms.push_back({x, 800});
    w.score += 1;
  }
}

int main() {
  // register event handlers
  global_event.register_handler("character_movement",
                                handle_character_movement);
  global_event.register_handler("character_collision",
                                handle_character_collision);
  global_event.register_handler("character_gravity", handle_character_gravity);
  global_event.register_handler("character_death", handle_character_death);
  global_event.register_handler("key_press", handle_key_press);
  global_event.register_handler("key_release", handle_key_release);
  global_event.register_handler("special_jump", handle_special_jump);
  global_event.register_handler("platform_movement", handle_platform_movement);
  // start receiver and sender
  std::thread event_receiver{receive_event};
  std::thread world_sender{send_world};
  // game loop
  timestamp previous = global_world.time;
  while (true) {
    {
      std::lock_guard<std::mutex> lock(global_mtx);
      // stop raising and applying events if the game is over
      if (global_world.game_over) {
        std::this_thread::yield();
        continue;
      }
      // calculate elapsed seconds
      timestamp present = timeline::get_time();
      float elapsed_seconds = (present - previous).count() * 0.000000001f;
      previous = present;
      // raise character movement event
      float velocity_x = 0.0;
      if (main_char.move_left) {
        velocity_x -= 120.0;
      }
      if (main_char.move_right) {
        velocity_x += 120.0;
      }
      global_event.raise_event(
          {"character_movement",
           {{"x", main_char.x + velocity_x * elapsed_seconds},
            {"y", main_char.y + main_char.velocity_y * elapsed_seconds}}});
      // raise platform movement event
      global_event.raise_event(
          {"platform_movement", {{"elapsed_seconds", elapsed_seconds}}});
      // raise collision event
      bool collision = false;
      float collision_level = 0.0;
      for (const auto& p : global_world.platforms) {
        if (main_char.x + 20 >= p.x && main_char.x <= p.x + 100 &&
            main_char.y + 20 >= p.y && main_char.y <= p.y - 10) {
          collision = true;
          collision_level = p.y;
          break;
        }
      }
      if (collision) {
        global_event.raise_event(
            {"character_collision", {{"collision_level", collision_level}}});
      } else {
        global_event.raise_event(
            {"character_gravity", {{"elapsed_seconds", elapsed_seconds}}});
      }
      // raise death event using script
      if (main_char.y > 800) {
        global_script.run_script("./src/script/raise_death_event.js");
      }
      // apply events
      global_event.apply_events(global_world, timeline::get_time());
    }
    std::this_thread::yield();
  }
}
