CXX = g++
CXXFLAGS = -I../ -I../include
CPPFLAGS = -Wall -ggdb -Wextra -Wno-sign-compare -O2
LDFLAGS = 
LIBS = 
TARGET = libcommon.a

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))

all: $(TARGET)

# $(TARGET): ../proto/liblmproto.a $(OBJ)
$(TARGET):  $(OBJ)
	ar -rvs -o $@ $(OBJ)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(CPPFLAGS)

# ../proto/liblmproto.a:
# 	$(MAKE) -C ../proto

.PHONY: all clean
clean:
	rm -f $(OBJ) $(TARGET)
