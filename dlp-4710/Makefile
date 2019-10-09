all: set-projector-power

#.PHONY: cyusbserial dlpc347x set-projector-power

cyusbserial: cyusbserial/libcyusbserial.la
cyusbserial/libcyusbserial.la:
	$(MAKE) -C cyusbserial

dlpc347x: cyusbserial dlpc347x/libdlpc347x.la
dlpc347x/libdlpc347x.la: cyusbserial/libcyusbserial.la 
	$(MAKE) -C dlpc347x

set-projector-power: cyusbserial dlpc347x set-projector-power/set-projector-power
set-projector-power/set-projector-power: dlpc347x/libdlpc347x.la cyusbserial/libcyusbserial.la
	$(MAKE) -C set-projector-power

install:
	$(MAKE) -C cyusbserial install
	$(MAKE) -C dlpc347x install
	$(MAKE) -C set-projector-power install

clean:
	$(MAKE) -C cyusbserial clean
	$(MAKE) -C dlpc347x clean
	$(MAKE) -C set-projector-power clean
