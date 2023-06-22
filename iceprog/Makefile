include ../config.mk

ifeq ($(OS),macos)
$(info Compiling for macOS, expects libsub installed via Homebrew.)

CFLAGS += -I /opt/homebrew/include
LDFLAGS += -L /opt/homebrew/lib -lusb-1.0

else #ifeq ($(OS),macos)

ifeq ($(STATIC),1)
LDFLAGS += -static
LDLIBS += $(shell for pkg in libftdi1 libftdi; do $(PKG_CONFIG) --silence-errors --static --libs $$pkg && exit; done; echo -lftdi; )
CFLAGS += $(shell for pkg in libftdi1 libftdi; do $(PKG_CONFIG) --silence-errors --static --cflags $$pkg && exit; done; )
else
LDLIBS += $(shell for pkg in libftdi1 libftdi; do $(PKG_CONFIG) --silence-errors --libs $$pkg && exit; done; echo -lftdi; )
CFLAGS += $(shell for pkg in libftdi1 libftdi; do $(PKG_CONFIG) --silence-errors --cflags $$pkg && exit; done; )

LDLIBS += -lusb-1.0
endif #ifeq ($(STATIC),1)

endif #ifeq ($(OS),macos)

all: $(PROGRAM_PREFIX)iceprog$(EXE)

ifeq ($(OS),macos)
$(PROGRAM_PREFIX)iceprog$(EXE): iceprog.o rpi_pico_interface.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)
else
$(PROGRAM_PREFIX)iceprog$(EXE): iceprog.o mpsse.o ftdi_interface.o rpi_pico_interface.o
	$(CC) -o $@ $(LDFLAGS) $^ $(LDLIBS)
endif

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
