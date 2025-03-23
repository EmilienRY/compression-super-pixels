CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC = src/ImageBase.cpp src/watershed.cpp
OBJ = $(SRC:.cpp=.o)

TARGET = watershed

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean
