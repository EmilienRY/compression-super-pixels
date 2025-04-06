CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC_A = src/ImageBase.cpp src/watershed.cpp
SRC_B = src/ImageBase.cpp src/slic.cpp

OBJ_A = $(SRC_A:.cpp=.o)
OBJ_B = $(SRC_B:.cpp=.o)

TARGET_A = watershed
TARGET_B = slic

all: $(TARGET_A) $(TARGET_B)

$(TARGET_A): $(OBJ_A)
	$(CXX) $(CXXFLAGS) -o $(TARGET_A) $(OBJ_A)

$(TARGET_B): $(OBJ_B)
	$(CXX) $(CXXFLAGS) -o $(TARGET_B) $(OBJ_B)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_A) $(OBJ_B) $(TARGET_A) $(TARGET_B)

.PHONY: all clean