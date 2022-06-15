CC=g++
CFLAGS = -Wall -std=c++1 -g
OBJECTS = sockets contener
SOURCES = sockets.cpp container.cpp

TAR=tar
TARFLAGS=-cvf
TARNAME=ex5.tar
TARSRCS=$(SOURCES) Makefile README

all: $(OBJECTS)

$(OBJECTS): %: %.cpp
        $(CC) $(CFLAGS) -o $@ $<
        
tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)