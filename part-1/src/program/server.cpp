#include <v8/libplatform/libplatform.h>
#include <v8/v8.h>

#include <SFML/Window.hpp>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <mutex>
#include <nlohmann/json.hpp>
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
    timeline::get_time(),
    {100, 300, false, false, 0},
    {{60, 500}, {260, 400}, {660, 200}, {460, 100}},
    {460, 300},
    {505, 90},
    false,
};
script_manager global_script{global_event, global_world};

auto& main_char = global_world.main_character;

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
}

void handle_character_gravity(world& w, const event& e) {
  main_char.velocity_y +=
      300.0 * std::get<double>(e.param.at("elapsed_seconds"));
}

void handle_character_death(world& w, const event& e) {
  main_char.x = 100;
  main_char.y = 300;
  main_char.velocity_y = 0;
}

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

void handle_game_finish(world& w, const event& e) {
  // handle the event using script
  global_script.run_script("./src/script/handle_finish_event.js");
}

void handle_special_jump(world& w, const event& e) {
  // special jump allows the character to jump in the air
  main_char.velocity_y -= 250.0;
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
  global_event.register_handler("game_finish", handle_game_finish);
  global_event.register_handler("special_jump", handle_special_jump);
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
      // update moving platform position using script
      global_script.run_script("./src/script/update_moving_platform.js");
      // raise movement event
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
      // raise collision event
      bool collision = false;
      for (const auto& p : global_world.platforms) {
        if (main_char.x + 20 >= p.x && main_char.x <= p.x + 100 &&
            main_char.y + 20 >= p.y && main_char.y <= p.y - 10) {
          collision = true;
          break;
        }
      }
      if (main_char.x + 20 >= global_world.moving_platform.x &&
          main_char.x <= global_world.moving_platform.x + 100 &&
          main_char.y + 20 >= global_world.moving_platform.y &&
          main_char.y <= global_world.moving_platform.y - 10) {
        collision = true;
      }
      if (collision) {
        global_event.raise_event({"character_collision", {}});
      } else {
        global_event.raise_event(
            {"character_gravity", {{"elapsed_seconds", elapsed_seconds}}});
      }
      // raise death event using script
      if (main_char.y > 600) {
        global_script.run_script("./src/script/raise_death_event.js");
      }
      // raise finish event
      const auto& f = global_world.finish_point;
      if (main_char.x + 20 >= f.x && main_char.x <= f.x + 10 &&
          main_char.y + 20 >= f.y && main_char.y <= f.y + 10) {
        global_event.raise_event({"game_finish", {}});
      }
      // apply events
      global_event.apply_events(global_world, timeline::get_time());
    }
    std::this_thread::sleep_for(milliseconds(1));
  }
}
