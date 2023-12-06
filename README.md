CSC 581 Homework 5
====
The final assignment is to implement a script management system, some scripted functionalities, improved input handling, and two new games to demonstrate the reusability of the game engine.

Prerequisites
----
* Ubuntu 20.04
    + Desktop system installed
    + Additional packages
        - `build-essential`
        - `libnode-dev`
        - `libsfml-dev`
        - `libzmq3-dev`
        - `nlohmann-json3-dev`
        - `x11-apps`

Getting Started
----
1. Enter the `part-1` directory of this project and run `make` in command line.
    * If succeed, two executables: `server` and `client` should appear in the `bin` directory.
2. Run `./bin/server` in the `part-1` directory from any terminal.
3. Run `./bin/client` in the `part-1` directory from a terminal which is opened from the Ubuntu desktop.
    * If succeed, a 800 x 600 window should appear and there will be a white square which represents the character.
    * Use the left and right direction keys to move the character.
    * Use the space key to jump.
    * When your character is dropped to the bottom, it is killed and respawned at the initial position.
    * The goal is to reach the yellow destination point.
    * (cheat) Use the combination of space key and down key to jump in the air.

The Second Game
----
1. Enter the `part-2` directory of this project and run `make` in command line.
    * If succeed, two executables: `server` and `client` should appear in the `bin` directory.
2. Run `./bin/server` in the `part-2` directory from any terminal.
3. Run `./bin/client` in the `part-2` directory from a terminal which is opened from the Ubuntu desktop.
    * If succeed, a 600 x 800 window should appear and there will be a white square which represents the character.
    * Use the left and right direction keys to move the character.
    * Use the space key to jump.
    * When your character is dropped to the bottom, it is killed and the game is over.
    * The goal is to survive as long as possible.

The Third Game
----
1. Enter the `part-3` directory of this project and run `make` in command line.
    * If succeed, two executables: `server` and `client` should appear in the `bin` directory.
2. Run `./bin/server` in the `part-3` directory from any terminal.
3. Run `./bin/client` in the `part-3` directory from a terminal which is opened from the Ubuntu desktop.
    * If succeed, a 800 x 600 window should appear and there will be a white square which represents the character.
    * Use the `S` key to start the game.
    * Use the space key to jump in the air.
    * When your character touches the blue pipes, is dropped to the bottom, or is going too high, it is killed and the game is over.
    * The goal is to survive as long as possible.
