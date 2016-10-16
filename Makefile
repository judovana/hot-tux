# where to install this program
PREFIX := /usr/local
VERSION := 0.2.2

# optimization cflags
CFLAGS += -O2 -Wall -g `pkg-config gdk-2.0 gdk-pixbuf-2.0  --cflags` -DPREFIX=\"$(PREFIX)\" -DVERSION=\"$(VERSION)\"

OBJS = hot-babe.o loader.o
CC = gcc
LIBS = `pkg-config gdk-2.0 gdk-pixbuf-2.0  --libs`

BIN_NAME = hot-tux

DOC = ChangeLog NEWS TODO LICENSE CONTRIBUTORS copyright config.example

all: $(BIN_NAME)

$(BIN_NAME): $(OBJS)
	cp hot-babe.1 $(BIN_NAME).1
	cp hot-babe.xpm $(BIN_NAME).xpm
	$(CC) -o $(BIN_NAME) $(OBJS) $(LIBS)

clean:
	rm $(BIN_NAME) *.o $(BIN_NAME).1 $(BIN_NAME).xpm

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 $(BIN_NAME) $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/$(BIN_NAME)/hb01
	install -m 0644 hb01/* $(DESTDIR)$(PREFIX)/share/$(BIN_NAME)/hb01
	install -d $(DESTDIR)$(PREFIX)/share/doc/$(BIN_NAME)
	install -m 0644 $(DOC) $(DESTDIR)$(PREFIX)/share/doc/$(BIN_NAME)
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 0644 $(BIN_NAME).1 $(DESTDIR)$(PREFIX)/share/man/man1
	install -d $(DESTDIR)$(PREFIX)/share/pixmaps
	install -m 0644 $(BIN_NAME).xpm $(DESTDIR)$(PREFIX)/share/pixmaps

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN_NAME)
	rm -rf $(DESTDIR)$(PREFIX)/share/$(BIN_NAME)
	rm -rf $(DESTDIR)$(PREFIX)/share/doc/$(BIN_NAME)
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/$(BIN_NAME).1
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/$(BIN_NAME).xpm
