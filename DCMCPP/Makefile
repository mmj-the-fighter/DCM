CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -g

INCLUDES = -I.

SOURCES = main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

EXEC = dcmcpp

all: $(EXEC)

$(EXEC): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(EXEC)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXEC)

.PHONY: all clean

