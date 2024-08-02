# Compiler and linker flags
CXX = g++
CXXFLAGS = -I$(SDL2_INCLUDE_PATH) -I$(SDL2_TTF_INCLUDE_PATH) -Wall -std=c++20
LDFLAGS = -L$(SDL2_LIB_PATH) -L$(SDL2_TTF_LIB_PATH) -lSDL2 -lSDL2_ttf

# Paths to SDL2 and SDL2_ttf
SDL2_INCLUDE_PATH = C:\DEVELOPMENT\SDLNORMAL\SDLFULL\include
SDL2_LIB_PATH = C:\DEVELOPMENT\SDLNORMAL\SDLFULL\lib
SDL2_TTF_INCLUDE_PATH = C:\DEVELOPMENT\SDLNORMAL\SDLFULL\include
SDL2_TTF_LIB_PATH = C:\DEVELOPMENT\SDLNORMAL\SDLFULL\lib

# Source files
SRCS = main.cpp 

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = main 

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile the source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJS) $(TARGET)

# Run the program
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
