CXX = g++
CXXFLAGS = -I.
CPPFLAGS = -Wall -ggdb
LDFLAGS = 
LIBS = 

SERVER_PROTO_FILES := $(wildcard *.proto)
COMM_PROTO_FILES := $(wildcard communication/*.proto)
COPIED_SERVER_PROTO_FILES := $(SERVER_PROTO_FILES:%.proto=communication/%.proto)

all: liblmproto.a 

liblmproto.a: $(COPIED_SERVER_PROTO_FILES) $(COMM_PROTO_FILES)
	$(MAKE) -C communication

$(COPIED_SERVER_PROTO_FILES):communication/%.proto : %.proto
	cp $^ $@

.PHONY: clean
clean:
	rm -f $(COPIED_SERVER_PROTO_FILES)
	$(MAKE) clean -C communication
