CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Iinclude

SRC = src/main.cpp \
      src/storage.cpp \
      src/log.cpp \
	  src/wal.cpp \
	  src/raft.cpp

TARGET = kvstore

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)