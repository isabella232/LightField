BUILD               = debug
PREFIX              = /usr/local

LIBTOOL             = libtool
SUDO                = sudo

CC                  = $(LIBTOOL) --mode=compile gcc -static
INSTALL             = $(LIBTOOL) --mode=install install
LD                  = $(LIBTOOL) --mode=link gcc -static
RM                  = $(LIBTOOL) --mode=clean rm

TARGET              = libcyusbserial.la
SOURCES             = cyi2c.c cyjtag.c cymisc.c cyphdc.c cyspi.c cyuart.c cyusb.c
OBJECTS             = $(SOURCES:.c=.lo)

CFLAGS              = -std=gnu17 -Werror -Wall -Wextra -pedantic
LFLAGS              = 

ifeq ($(BUILD),debug)
CFLAGS             += -D_DEBUG -g -Og
LFLAGS             += -g
else ifeq ($(BUILD),release)
CFLAGS             += -DNDEBUG -O3
LFLAGS             += -s
endif

all: $(TARGET)

%.lo: %.c
	@#echo Compiling $<
	$(CC) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	@#echo Linking $@
	$(LD) $(LFLAGS) -o $@ $^ -lpthread -lusb-1.0

install: $(TARGET)
	@#$(SUDO) $(INSTALL) -D -m 755 $(TARGET) $(PREFIX)/lib
	@#$(SUDO) $(INSTALL) -D -m 644 CyUSBSerial.h $(PREFIX)/include
	@#$(SUDO) $(LIBTOOL) --mode=finish $(PREFIX)/lib

clean:
	-$(RM) $(TARGET) $(OBJECTS) 

## Dependencies

cyi2c.lo:  cyi2c.c  CyUSBCommon.h CyUSBSerial.h
cyjtag.lo: cyjtag.c CyUSBCommon.h CyUSBSerial.h
cymisc.lo: cymisc.c CyUSBCommon.h CyUSBSerial.h
cyphdc.lo: cyphdc.c CyUSBCommon.h CyUSBSerial.h
cyspi.lo:  cyspi.c  CyUSBCommon.h CyUSBSerial.h
cyuart.lo: cyuart.c CyUSBCommon.h CyUSBSerial.h
cyusb.lo:  cyusb.c  CyUSBCommon.h CyUSBSerial.h
