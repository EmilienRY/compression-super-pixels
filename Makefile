CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

SRC_A = src/ImageBase.cpp src/watershed.cpp
SRC_B = src/ImageBase.cpp src/slic.cpp
SRC_C = src/ImageBase.cpp src/compressionPalette.cpp
SRC_D = src/ImageBase.cpp src/psnr.cpp

OBJ_A = $(SRC_A:.cpp=.o)
OBJ_B = $(SRC_B:.cpp=.o)
OBJ_C = $(SRC_C:.cpp=.o)
OBJ_D = $(SRC_D:.cpp=.o)

TARGET_A = watershed
TARGET_B = slic
TARGET_C = compressionPalette
TARGET_D = psnr

all: $(TARGET_A) $(TARGET_B) $(TARGET_C) $(TARGET_D)

$(TARGET_A): $(OBJ_A)
	$(CXX) $(CXXFLAGS) -o $(TARGET_A) $(OBJ_A)

$(TARGET_B): $(OBJ_B)
	$(CXX) $(CXXFLAGS) -o $(TARGET_B) $(OBJ_B)

$(TARGET_C): $(OBJ_C)
	$(CXX) $(CXXFLAGS) -o $(TARGET_C) $(OBJ_C)
$(TARGET_D): $(OBJ_D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_D) $(OBJ_D)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_A) $(OBJ_B) $(OBJ_C) $(OBJ_D) $(TARGET_A) $(TARGET_B) $(TARGET_C) $(TARGET_D)

.PHONY: all clean