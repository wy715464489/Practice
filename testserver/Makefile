CXX = g++
CXXFLAGS = -I../  -I.
CPPFLAGS = -Wall -ggdb -Werror
LDFLAGS = -L../lib -L../common -L../net
LIBS =  -lpthread -ldl -lrt -lcommon -lnet 

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
TARGET = testserver
VERSION = 0.5.0

all: $(TARGET)

$(TARGET): ../net/libnet.a ../common/libcommon.a  $(OBJ)
	$(CXX) -o $@ $(OBJ) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

../common/libcommon.a:
	$(MAKE) -C ../common

../net/libnet.a: 
	$(MAKE) -C ../net

.PHONY: all clean

clean:
	rm -f $(OBJ) $(TARGET) version.h
