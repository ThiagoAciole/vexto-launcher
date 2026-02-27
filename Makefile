# Makefile for oneLauncher
# Precise setup for XFCE Panel detection

NAME = vexto-app-launcher
SONAME = libvexto-app-launcher.so
SRC = src/core/main.c src/components/window.c src/components/grid.c src/utils/style.c
OBJ = $(SRC:.c=.o)

CC = gcc
CFLAGS = -Wall -g -fPIC `pkg-config --cflags gtk+-3.0 libxfce4panel-2.0 libxfce4ui-2 libxfce4util-1.0 gio-2.0`
LDFLAGS = -shared `pkg-config --libs gtk+-3.0 libxfce4panel-2.0 libxfce4ui-2 libxfce4util-1.0 gio-2.0`

PREFIX ?= /usr
DESTDIR ?=

# Better path detection for multiarch
MULTIARCH = $(shell gcc -print-multiarch 2>/dev/null)
ifdef MULTIARCH
    LIBDIR = $(PREFIX)/lib/$(MULTIARCH)/xfce4/panel/plugins
else
    LIBDIR = $(PREFIX)/lib/xfce4/panel/plugins
endif

DATADIR = $(PREFIX)/share/xfce4/panel/plugins
ICONDIR = $(PREFIX)/share/icons/hicolor/48x48/apps
ASSETDIR = $(PREFIX)/share/vexto-app-launcher/icons
STYLEDIR = $(PREFIX)/share/vexto-app-launcher/styles

all: $(SONAME) data/vexto-app-launcher.desktop

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(SONAME): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

data/vexto-app-launcher.desktop: data/vexto-app-launcher.desktop.in
	cp data/vexto-app-launcher.desktop.in data/vexto-app-launcher.desktop

clean:
	rm -f $(OBJ) $(SONAME) data/vexto-app-launcher.desktop

install: all
	mkdir -p $(DESTDIR)$(LIBDIR)
	mkdir -p $(DESTDIR)$(DATADIR)
	mkdir -p $(DESTDIR)$(ICONDIR)
	mkdir -p $(DESTDIR)$(ASSETDIR)
	mkdir -p $(DESTDIR)$(STYLEDIR)
	install -m 755 $(SONAME) $(DESTDIR)$(LIBDIR)/
	install -m 644 data/vexto-app-launcher.desktop $(DESTDIR)$(DATADIR)/
	install -m 644 data/icons/vexto-app-launcher.svg $(DESTDIR)$(ICONDIR)/
	install -m 644 data/styles/main.css $(DESTDIR)$(STYLEDIR)/
	gtk-update-icon-cache -f -t $(DESTDIR)$(PREFIX)/share/icons/hicolor || true

uninstall:
	rm -f $(DESTDIR)$(LIBDIR)/$(SONAME)
	rm -f $(DESTDIR)$(DATADIR)/vexto-app-launcher.desktop
	rm -f $(DESTDIR)$(ICONDIR)/vexto-app-launcher.svg
	rm -rf $(DESTDIR)$(ASSETDIR)
	rm -rf $(DESTDIR)$(STYLEDIR)

.PHONY: all clean install uninstall
