BINS = bin/server bin/client
OBJS = src/event/event_manager.o src/event/event.o src/event/timeline.o src/event/timestamp.o src/script_manager/script_manager.o src/world/character.o src/world/obstacle.o src/world/platform.o src/world/world.o 
LIBS = -lpthread -lsfml-graphics -lsfml-system -lsfml-window -lv8 -lzmq
CXXFLAGS = -std=c++17 -g -Wall -I/usr/include/v8 -Iinclude

all: $(BINS)

bin/server: $(OBJS) src/program/server.o
	$(CXX) -o bin/server $(OBJS) src/program/server.o  $(LIBS)

bin/client: $(OBJS) src/program/client.o
	$(CXX) -o bin/client $(OBJS) src/program/client.o  $(LIBS)

####

src/event/event_manager.o: include/event/event_manager.hpp include/event/timeline.hpp

src/event/event.o: include/event/event.hpp include/event/timeline.hpp

src/event/timeline.o: include/event/timeline.hpp

src/event/timestamp.o: include/event/timestamp.hpp

src/program/client.o: include/event/event.hpp include/world/world.hpp

src/program/server.o: include/event/event_manager.hpp include/event/timeline.hpp include/script_manager/script_manager.hpp include/world/world.hpp

src/script_manager/script_manager.o: include/script_manager/script_manager.hpp include/event/timeline.hpp

src/world/character.o: include/world/character.hpp

src/world/obstacle.o: include/world/obstacle.hpp

src/world/platform.o: include/world/platform.hpp

src/world/world.o: include/world/world.hpp

####

.PHONY: clean
clean:
	rm $(BINS) $(OBJS)
