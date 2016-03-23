CXX = g++
CXXFLAGS = -I../ -I../include -I./include
CPPFLAGS = -Wall -ggdb -Wextra -fpermissive -Wno-sign-compare -O2
LDFLAGS = -L./lib/
LIBS = -lgtest -pthread
TARGET = test

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))

all: $(OBJ)
	g++ -o $(TARGET) $(OBJ) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: all clean
clean:
	rm -f $(OBJ) $(TARGET)