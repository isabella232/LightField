BUILD    = release
PREFIX   = /usr/local

LIBTOOL  = libtool
SUDO     = sudo

CC       = $(LIBTOOL) --mode=compile gcc
INSTALL  = $(LIBTOOL) --mode=install install
LD       = $(LIBTOOL) --mode=link gcc -static
RM       = $(LIBTOOL) --mode=clean rm

TARGET   = libdlpc347x.la
SOURCES  = dlpc347x_dual.c dlpc347x_internal_patterns.c dlpc_common.c
OBJECTS  = $(SOURCES:.c=.lo)

CFLAGS   = -std=gnu17 -Werror -Wall -Wextra -pedantic -I../cyusbserial
LFLAGS   = -L../cyusbserial

ifeq ($(BUILD),debug)
CFLAGS  += -D_DEBUG -g -Og
LFLAGS  += -g
else ifeq ($(BUILD),release)
CFLAGS  += -DNDEBUG -O3
LFLAGS  += -s
endif

all: $(TARGET)

%.lo: %.c
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $^ -lcyusbserial

install: $(TARGET)
	@#$(SUDO) $(INSTALL) -D -m 644 dlpc347x_dual.h dlpc347x_internal_patterns.h dlpc_common.h $(PREFIX)/include
	@#$(SUDO) $(INSTALL) -D -m 755 $(TARGET) $(PREFIX)/lib
	@#$(SUDO) $(LIBTOOL) --mode=finish $(PREFIX)/lib

clean:
	-$(RM) $(TARGET) $(OBJECTS) 

## Dependencies

dlpc347x.lo:                   dlpc347x.c dlpc347x.h dlpc_common.h dlpc_common_private.h
dlpc347x_dual.lo:              dlpc347x_dual.c dlpc347x_dual.h dlpc_common.h dlpc_common_private.h
dlpc347x_internal_patterns.lo: dlpc347x_internal_patterns.c dlpc347x_internal_patterns.h dlpc_common.h
dlpc_common.lo:                dlpc_common.c dlpc_common.h
