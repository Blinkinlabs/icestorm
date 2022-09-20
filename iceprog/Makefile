include ../config.mk

ifeq ($(STATIC),1)
LDFLAGS += -static
else

# CFLAGS += -llibusb-1.0
LDLIBS += -lusb-1.0
endif
#LDLIBS += -framework IOKit -framework CoreFoundation -framework AppKit

all: $(PROGRAM_PREFIX)iceprog$(EXE)

$(PROGRAM_PREFIX)iceprog$(EXE): iceprog.o rpi_pico_interface.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(PROGRAM_PREFIX)iceprog$(EXE) $(DESTDIR)$(PREFIX)/bin/$(PROGRAM_PREFIX)iceprog$(EXE)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(PROGRAM_PREFIX)iceprog$(EXE)

clean:
	rm -f $(PROGRAM_PREFIX)iceprog
	rm -f $(PROGRAM_PREFIX)iceprog.exe
	rm -f *.o *.d

-include *.d

.PHONY: all install uninstall clean
