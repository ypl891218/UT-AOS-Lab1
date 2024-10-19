# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -O2

# Source files
SRC = lab1.cpp perf_event_open.cpp 
SRC_2 = main_mem_access.cpp mem_access.cpp perf_event_open.cpp 

# Header files
HEADERS = perf_event_open.hpp
HEADERS_2 = perf_event_open.hpp mem_access.hpp

# Output executable
TARGET = lab1
TARGET_2 = main_mem_access

# Default rule to build the executable
all: $(TARGET) $(TARGET_2)

# Rule to link object files and generate the executable
$(TARGET): $(SRC) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

$(TARGET_2): $(SRC_2) $(HEADERS_2)
	$(CXX) $(CXXFLAGS) $(SRC_2) -o $(TARGET_2)

# Rule to clean build artifacts
clean:
	rm -f $(TARGET) $(TARGET_2) *.o

