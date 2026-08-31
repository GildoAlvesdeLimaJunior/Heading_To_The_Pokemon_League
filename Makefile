CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude
TARGET   = pokemon_rpg
SRCS     = src/main.cpp src/MapParser.cpp src/GraphEngine.cpp \
           src/StateEngine.cpp src/BattleEngine.cpp src/RNG.cpp src/GUI.cpp
OBJS     = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) data/graph.txt

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all run clean
