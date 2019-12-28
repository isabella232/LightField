BUILD               = release
PREFIX              = /usr/local

LIBTOOL             = libtool
SUDO                = sudo

CC                  = $(LIBTOOL) --mode=compile gcc
INSTALL             = $(LIBTOOL) --mode=install install
LD                  = $(LIBTOOL) --mode=link gcc
RM                  = $(LIBTOOL) --mode=clean rm

TARGET              = set-projector-power
SOURCES             = set-projector-power.c cypress_i2c.c
OBJECTS             = $(SOURCES:.c=.lo)

CFLAGS              = -std=gnu17 -Wall -Wextra -Wno-unused-parameter -Werror -pedantic -I../cyusbserial -I../dlpc347x
LFLAGS              = -L../cyusbserial -L../dlpc347x

ifeq ($(BUILD),debug)
CFLAGS             += -D_DEBUG -g -Og
LFLAGS             += -g
else ifeq ($(BUILD),release)
CFLAGS             += -DNDEBUG -O3
LFLAGS             += -s
endif

all: $(TARGET)

%.lo: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $^ -lcyusbserial -ldlpc347x

install: $(TARGET)
	$(SUDO) $(INSTALL) -D -m 755 $(TARGET) $(PREFIX)/bin

clean:
	-$(RM) $(TARGET) $(OBJECTS) 

## Dependencies

set-projector-power.lo: set-projector-power.c cypress_i2c.h
cypress_i2c.lo: cypress_i2c.c
