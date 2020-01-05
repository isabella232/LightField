BUILD               = debug
PREFIX              = /usr/local

LIBTOOL             = libtool
SUDO                = sudo

CXX                 = $(LIBTOOL) --mode=compile g++
INSTALL             = $(LIBTOOL) --mode=install install
LD                  = $(LIBTOOL) --mode=link g++
RM                  = $(LIBTOOL) --mode=clean rm

TARGET              = set-projector-power
SOURCES             = set-projector-power.cpp
OBJECTS             = $(SOURCES:.cpp=.lo)

CFLAGS              = -std=gnu++17 -Wall -Wextra -Wno-unused-parameter -Werror -pedantic
LFLAGS              = 

ifeq ($(BUILD),debug)
CFLAGS             += -D_DEBUG -g -Og
LFLAGS             += -g
else ifeq ($(BUILD),release)
CFLAGS             += -DNDEBUG -O3
LFLAGS             += -s
endif

all: $(TARGET)

%.lo: %.cpp
	$(CXX) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $^

install: $(TARGET)
	$(SUDO) $(INSTALL) -D -m 755 $(TARGET) $(PREFIX)/bin

clean:
	-$(RM) $(TARGET) $(OBJECTS) 

## Dependencies

set-projector-power.lo: set-projector-power.cpp
