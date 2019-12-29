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

CFLAGS              = -std=gnu++17 -Wall -Wextra -Wno-unused-parameter -Werror -pedantic -I../libMPSSE__0-6_Source/Release/include -I../libMPSSE__0-6_Source/Release/include/linux
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
	$(LD) $(LFLAGS) -o $@ $^ ../libMPSSE__0-6_Source/LibMPSSE/Build/Linux/libMPSSE.a -ldl

install: $(TARGET)
	$(SUDO) $(INSTALL) -D -m 755 $(TARGET) $(PREFIX)/bin

clean:
	-$(RM) $(TARGET) $(OBJECTS) 

## Dependencies

set-projector-power.lo: set-projector-power.cpp
