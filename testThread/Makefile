CXX = g++
CXXFLAGS = -I../ 
CPPFLAGS = -Wall -ggdb
LDFLAGS = -L../net -L../common -L../proto 
LIBS = -lnet -lcommon  -lpthread

SRC1 = $(wildcard *.cpp)
SRC2 = $(filter-out recruit.cpp, $(SRC1))
SRC = $(filter-out rob_start.cpp, $(SRC2))
OBJ = $(patsubst %.cpp,%.o,$(SRC))
TARGETS = $(SRC:%.cpp=%)

all: $(TARGETS)

$(TARGETS):%:%.cpp ../net/libnet.a  ../common/libcommon.a
	g++ -o $@ -ggdb -Wall $< $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: all clean
clean:
	rm -f $(TARGETS)
