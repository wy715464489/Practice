CXXFLAGS = -I.
CPPFLAGS = -Wall -ggdb
LDFLAGS = 
LIBS = 
CXX = g++

TARGET = ./liblmproto.a

PROTO_FILES := $(wildcard *.proto)
PB_CC_FILES := $(PROTO_FILES:%.proto=%.pb.cc)
OBJS := $(PROTO_FILES:%.proto=%.pb.o)

# TODO(Weitong): 只编译修改过的proto
all: $(TARGET)
#all:
#	echo $(PROTO_FILES)
$(TARGET): $(OBJS)
	$(CXX) -c -o $@ $(OBJS)

%.pb.o: %.pb.cc proto_built
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(CPPFLAGS)

proto_built: $(PB_CC_FILES)

%.pb.cc: %.proto
	protoc $< --cpp_out=./


.PHONY: clean proto_built
clean:
	rm -f *.pb.h *.pb.cc *.o ../liblmproto.a
