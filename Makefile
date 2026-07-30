CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude

SRC = src/main.cpp \
      src/storage.cpp \
      src/log.cpp \
	  src/wal.cpp \
	  src/raft.cpp \
	  src/transport.cpp

TARGET = bin/kvstore

all: bin
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

bin:
	mkdir -p bin

clean:
	rm -f $(TARGET)