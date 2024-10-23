# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

# Source files
SRC = task12.cpp perf_event_open.cpp utils.cpp
SRC_2 = task3.cpp mem_access.cpp perf_event_open.cpp utils.cpp
SRC_4 = task4.cpp mem_access.cpp perf_event_open.cpp utils.cpp 

# Header files
HEADERS = perf_event_open.hpp utils.hpp
HEADERS_2 = perf_event_open.hpp mem_access.hpp utils.hpp 
HEADERS_4 = perf_event_open.hpp mem_access.hpp utils.hpp

# Output executable
TARGET = task12
TARGET_2 = task3
TARGET_4 = task4

# Default rule to build the executable
all: $(TARGET) $(TARGET_2) $(TARGET_4)

# Rule to link object files and generate the executable
$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(TARGET_2): $(SRC_2) $(HEADERS_2)
	$(CXX) $(CXXFLAGS) $(SRC_2) -o $(TARGET_2)

$(TARGET_4): $(SRC_4) $(HEADERS_4)
	$(CXX) $(CXXFLAGS) $(SRC_4) -o $(TARGET_4)

# Rule to clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET_2) $(TARGET_4) *.o

