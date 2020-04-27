BUILD               = debug

CXX                 = g++
LD                  = g++
RM                  = rm

TARGET              = set-projector-power
SOURCES             = dlpc350_usb.cpp dlpc350_api.cpp main.cpp
OBJECTS             = $(SOURCES:.cpp=.o)

CFLAGS              = -std=gnu++17 -Wall -Wextra -Werror -pedantic
LFLAGS              =
ifeq ($(BUILD),debug)
CFLAGS             += -D_DEBUG -g -Og
LFLAGS             += -g
else ifeq ($(BUILD),release)
CFLAGS             += -DNDEBUG -O3
LFLAGS             += -s
endif

all: $(TARGET)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

$(TARGET): $(OBJECTS)
	$(LD) $(LFLAGS) -o $@ $^ -lhidapi-libusb

clean:
	$(RM) -f $(TARGET) $(OBJECTS)

## Dependencies

dlpc350_usb.o: dlpc350_usb.cpp
dlpc350_api.o: dlpc350_api.cpp
main.o: main.cpp
