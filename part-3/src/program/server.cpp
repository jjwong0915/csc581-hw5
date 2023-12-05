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
    timeline::get_time(), {385, 285, false, false, 0}, {}, {}, 0, false, false,
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
  w.game_over = true;
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
    main_char.velocity_y -= 250.0;
  } else if (key == sf::Keyboard::S) {
    w.game_start = true;
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

void handle_obstacle_movement(world& w, const event& e) {
  for (auto& o : w.obstacles) {
    o.x -= 100.0 * std::get<double>(e.param.at("elapsed_seconds"));
  }
  // remove platform go over top
  while (w.obstacles.size() > 0 && w.obstacles.front().x < -40) {
    w.obstacles.pop_front();
  }
  // add plaform from bottom
  if (w.obstacles.empty() || w.obstacles.back().x < 400) {
    float y = std::uniform_real_distribution<float>(0, 400)(re);
    w.obstacles.push_back({800, 0, 40, y});
    w.obstacles.push_back({800, y + 200, 40, 800 - (y + 200)});
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
  global_event.register_handler("obstacle_movement", handle_obstacle_movement);
  // start receiver and sender
  std::thread event_receiver{receive_event};
  std::thread world_sender{send_world};
  // game loop
  timestamp previous = global_world.time;
  while (true) {
    {
      std::lock_guard<std::mutex> lock(global_mtx);
      // calculate elapsed seconds
      timestamp present = timeline::get_time();
      float elapsed_seconds = (present - previous).count() * 0.000000001f;
      previous = present;
      // raise events if the game is start and not over
      if (global_world.game_start && !global_world.game_over) {
        // raise character movement event
        float velocity_x = 0.0;
        global_event.raise_event(
            {"character_movement",
             {{"x", main_char.x + velocity_x * elapsed_seconds},
              {"y", main_char.y + main_char.velocity_y * elapsed_seconds}}});
        // raise obstacle movement event
        global_event.raise_event(
            {"obstacle_movement", {{"elapsed_seconds", elapsed_seconds}}});
        // raise collision event
        bool collision = false;
        float collision_level = 0.0;
        for (const auto& o : global_world.obstacles) {
          if (main_char.x + 30 >= o.x && main_char.x <= o.x + o.w &&
              main_char.y + 30 >= o.y && main_char.y <= o.y + o.h) {
            collision = true;
            collision_level = o.y;
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
        if (main_char.y < -100 || main_char.y > 600) {
          global_script.run_script("./src/script/raise_death_event.js");
        }
      }
      // apply events
      global_event.apply_events(global_world, timeline::get_time());
    }
    std::this_thread::yield();
  }
}
