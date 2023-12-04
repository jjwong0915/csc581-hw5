BINS = bin/server bin/client
OBJS = src/event/event_manager.o src/event/event.o src/event/timeline.o src/event/timestamp.o src/script_manager/script_manager.o src/world/character.o src/world/finish.o src/world/platform.o src/world/world.o 
LIBS = -lpthread -lsfml-graphics -lsfml-system -lsfml-window -lv8 -lzmq
CXXFLAGS = -std=c++17 -g -Wall -I/usr/include/v8 -Iinclude

all: $(BINS)

bin/server: $(OBJS) src/program/server.o
	$(CXX) -o bin/server $(OBJS) src/program/server.o  $(LIBS)

bin/client: $(OBJS) src/program/client.o
	$(CXX) -o bin/client $(OBJS) src/program/client.o  $(LIBS)

####

.PHONY: clean
clean:
	rm $(BINS) $(OBJS)
