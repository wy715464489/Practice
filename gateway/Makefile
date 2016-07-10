CXX = g++
CXXFLAGS = -I..  -I.
CPPFLAGS = -Wall -ggdb -Wextra -Wno-sign-compare -O2 -DINTERNALIPSERVER
LDFLAGS = -L../net -L../common 
LIBS = -lnet -lcommon   -lpthread -ldl -lrt 

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
TARGET = gateway
VERSION = 0.5.0

all: $(TARGET)

$(TARGET): ../net/libnet.a  ../common/libcommon.a $(OBJ)
	g++ -o $@ $(OBJ) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

# version.h:
# 	../tools/create_version $(VERSION)

# %.o:%.cpp version.h
# 	g++ -c $< -o $@  $(CXXFLAGS) $(CPPFLAGS)

../common/libcommon.a:
	$(MAKE) -C ../common

../net/libnet.a: 
	$(MAKE) -C ../net

.PHONY: all clean

clean:
	rm -f $(OBJ) $(TARGET) version.h
