#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <set>
#include <zmq.hpp>

#include "event/event.hpp"
#include "world/world.hpp"

void send_event(zmq::socket_t& s, const event& e) {
  std::string event_json = json(e).dump();
  s.send(zmq::message_t{event_json.begin(), event_json.end()},
         zmq::send_flags::none);
  zmq::message_t dummy_message;
  zmq::recv_result_t recv_result;
  recv_result = s.recv(dummy_message);
}

int main() {
  std::set<sf::Keyboard::Key> pressed_key;
  // load font
  sf::Font f;
  f.loadFromFile("./font.ttf");
  // setup sockets
  zmq::context_t context;
  zmq::socket_t event_socket(context, ZMQ_REQ);
  event_socket.connect("tcp://127.0.0.1:5555");
  zmq::socket_t world_socket(context, ZMQ_REQ);
  world_socket.connect("tcp://127.0.0.1:5556");
  // create window
  sf::VideoMode mode{800, 600};
  sf::RenderWindow window(mode, "Homework 5", sf::Style::Close);
  while (window.isOpen()) {
    // handle window close
    sf::Event e;
    while (window.pollEvent(e)) {
      if (e.type == sf::Event::Closed) {
        window.close();
      } else if (e.type == sf::Event::KeyPressed) {
        sf::Keyboard::Key key = e.key.code;
        if (pressed_key.count(key) == 0) {
          send_event(event_socket, {"key_press", {{"key", (double)key}}});
          pressed_key.insert(key);
          // raise special jump event if the chord is pressed
          if (key == sf::Keyboard::Space || key == sf::Keyboard::Up) {
            if (pressed_key.count(sf::Keyboard::Space) > 0 &&
                pressed_key.count(sf::Keyboard::Down)) {
              send_event(event_socket, {"special_jump", {}});
            }
          }
        }
      } else if (e.type == sf::Event::KeyReleased) {
        sf::Keyboard::Key key = e.key.code;
        if (pressed_key.count(key) > 0) {
          send_event(event_socket, {"key_release", {{"key", (double)key}}});
          pressed_key.erase(key);
        }
      }
    }
    // request world data
    std::string data_req = json{}.dump();
    world_socket.send(zmq::message_t{data_req.begin(), data_req.end()},
                      zmq::send_flags::none);
    zmq::message_t world_data;
    if (world_socket.recv(world_data)) {
      // update game window
      window.clear();
      world world_object = json::parse(world_data.to_string());
      sf::RectangleShape main_character{{20, 20}};
      main_character.setPosition(world_object.main_character.x,
                                 world_object.main_character.y);
      window.draw(main_character);
      for (const auto& platform_object : world_object.platforms) {
        sf::RectangleShape platform_shape({100, 15});
        platform_shape.setPosition(platform_object.x, platform_object.y);
        platform_shape.setFillColor(sf::Color::Blue);
        window.draw(platform_shape);
      }
      sf::RectangleShape moving_platform({100, 15});
      moving_platform.setPosition(world_object.moving_platform.x,
                                  world_object.moving_platform.y);
      moving_platform.setFillColor(sf::Color::Blue);
      window.draw(moving_platform);
      sf::CircleShape finish_point(5);
      finish_point.setPosition(world_object.finish_point.x,
                               world_object.finish_point.y);
      finish_point.setFillColor(sf::Color::Yellow);
      window.draw(finish_point);
      if (world_object.finished) {
        sf::Text t{" You Win !!!", f, 40};
        t.setFillColor(sf::Color::Red);
        window.draw(t);
      }
      window.display();
    }
  }
}