#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include <set>
#include <string>
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
  sf::Font font;
  font.loadFromFile("./font.ttf");
  // setup sockets
  zmq::context_t context;
  zmq::socket_t event_socket(context, ZMQ_REQ);
  event_socket.connect("tcp://127.0.0.1:5555");
  zmq::socket_t world_socket(context, ZMQ_REQ);
  world_socket.connect("tcp://127.0.0.1:5556");
  // create window
  sf::VideoMode mode{600, 800};
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
      std::string text_content = "Score: " + std::to_string(world_object.score);
      if (world_object.game_over) {
        text_content += "\nGame Over";
      }
      sf::Text text{text_content, font, 40};
      text.setFillColor(sf::Color::Red);
      window.draw(text);
      window.display();
    }
  }
}