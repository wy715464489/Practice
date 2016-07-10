CXXFLAGS = -I../ -I/usr/local/include/
CPPFLAGS = -Wall -ggdb -Werror -Wextra -Wno-sign-compare -O2
LDFLAGS = 
LIBS = 

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
TARGET = libnet.a

all: $(TARGET)

#$(TARGET): ../proto/liblmproto.a $(OBJ) 
$(TARGET):  $(OBJ) 
	ar -rvs -o $(TARGET) $(OBJ)

.PHONY: all clean
clean:
	rm -f $(OBJ) $(TARGET)
