EXE     = gpio-event-handler
CFILE  = *.cpp
DEPPKGS = gio-unix-2.0 glib-2.0
CC      ?= $(CROSS_COMPILE)g++
CFLAGS += $(shell pkg-config --cflags $(DEPPKGS))
CFLAGS += -std=c++20
LIBS += $(shell pkg-config --libs $(LIBPKGS))
LIBS += -fPIC -lrt -lm -lsystemd -lsdbusplus -lphosphor_dbus

all: $(EXE)
$(EXE): $(CFILE)
	$(CC) -g  $(CFLAGS) -o $@ $^ -lstdc++ $(LIBS) 

clean:
	rm -f $(EXE) *.o *.d

distclean: clean
	rm -f *.cpp~ *.hpp~ *.sh~ Makefile~ config.mk~
