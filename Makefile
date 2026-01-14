# Compiler and Flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -pthread -Iinclude

# Source Files and Output
SRC = src/main.cpp src/WebServer.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = server

# Default Build Rule
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJ)

# Clean Rule
clean:
	rm -f src/*.o $(TARGET)