CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC_A = src/ImageBase.cpp src/watershed.cpp
SRC_B = src/ImageBase.cpp src/slic.cpp
SRC_C = src/ImageBase.cpp src/compression.cpp

OBJ_A = $(SRC_A:.cpp=.o)
OBJ_B = $(SRC_B:.cpp=.o)
OBJ_C = $(SRC_C:.cpp=.o)

TARGET_A = watershed
TARGET_B = slic
TARGET_C = compressionPalette

all: $(TARGET_A) $(TARGET_B) $(TARGET_C)

$(TARGET_A): $(OBJ_A)
	$(CXX) $(CXXFLAGS) -o $(TARGET_A) $(OBJ_A)

$(TARGET_B): $(OBJ_B)
	$(CXX) $(CXXFLAGS) -o $(TARGET_B) $(OBJ_B)

$(TARGET_C): $(OBJ_C)
	$(CXX) $(CXXFLAGS) -o $(TARGET_C) $(OBJ_C)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_A) $(OBJ_B) $(OBJ_C) $(TARGET_A) $(TARGET_B) $(TARGET_C)

.PHONY: all clean