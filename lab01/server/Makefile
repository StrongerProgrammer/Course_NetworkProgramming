CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra
TARGET = server

all: $(TARGET)

$(TARGET): server.cpp socket_utils.h
	$(CXX) $(CXXFLAGS) server.cpp -o $(TARGET)

clean:
	rm -f $(TARGET)