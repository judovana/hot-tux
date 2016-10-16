# where to install this program
PREFIX := /usr/local
VERSION := 0.2.2

# optimization cflags
CFLAGS += -O2 -Wall -g `pkg-config gdk-2.0 gdk-pixbuf-2.0  --cflags` -DPREFIX=\"$(PREFIX)\" -DVERSION=\"$(VERSION)\"

OBJS = hot-babe.o loader.o
CC = gcc
LIBS = `pkg-config gdk-2.0 gdk-pixbuf-2.0  --libs`

DOC = ChangeLog NEWS TODO LICENSE CONTRIBUTORS copyright config.example

all: hot-babe

hot-babe: $(OBJS)
	$(CC) -o hot-babe $(OBJS) $(LIBS)

clean:
	rm -f hot-babe *.o

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 0755 hot-babe $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)$(PREFIX)/share/hot-babe/hb01
	install -m 0644 hb01/* $(DESTDIR)$(PREFIX)/share/hot-babe/hb01
	install -d $(DESTDIR)$(PREFIX)/share/doc/hot-babe
	install -m 0644 $(DOC) $(DESTDIR)$(PREFIX)/share/doc/hot-babe
	install -d $(DESTDIR)$(PREFIX)/share/man/man1
	install -m 0644 hot-babe.1 $(DESTDIR)$(PREFIX)/share/man/man1
	install -d $(DESTDIR)$(PREFIX)/share/pixmaps
	install -m 0644 hot-babe.xpm $(DESTDIR)$(PREFIX)/share/pixmaps

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/hot-babe
	rm -rf $(DESTDIR)$(PREFIX)/share/hot-babe
	rm -rf $(DESTDIR)$(PREFIX)/share/doc/hot-babe
	rm -f $(DESTDIR)$(PREFIX)/share/man/man1/hot-babe.1
	rm -f $(DESTDIR)$(PREFIX)/share/pixmaps/hot-babe.xpm
