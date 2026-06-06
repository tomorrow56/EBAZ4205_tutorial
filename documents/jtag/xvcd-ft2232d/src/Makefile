# -----------------------------------------------------------------------
# Makefile for xvcd_ft2232d
#
# Linux (default):
#   make
#
# Windows cross-compile from Linux (MinGW-w64):
#   make CC=x86_64-w64-mingw32-gcc TARGET=xvcd_ft2232d.exe SRC=xvcd_ft2232d_win.c \
#        FTDI_PREFIX=/path/to/libftdi-win
#
# Windows native build (MinGW-w64 shell):
#   mingw32-make CC=gcc TARGET=xvcd_ft2232d.exe SRC=xvcd_ft2232d_win.c \
#               FTDI_CFLAGS="-I/msys64/mingw64/include/libftdi1" \
#               FTDI_LIBS="-lftdi1 -lusb-1.0 -lws2_32"
# -----------------------------------------------------------------------

CC      ?= gcc
SRC     ?= xvcd_ft2232d.c
TARGET  ?= xvcd_ft2232d

# Auto-detect pkg-config name (libftdi1 preferred, fall back to libftdi)
FTDI_CFLAGS ?= $(shell pkg-config --cflags libftdi1 2>/dev/null || \
                        pkg-config --cflags libftdi  2>/dev/null)
FTDI_LIBS   ?= $(shell pkg-config --libs   libftdi1 2>/dev/null || \
                        pkg-config --libs   libftdi  2>/dev/null)

CFLAGS  += -Wall -Wextra -Werror -Os -std=c99 $(FTDI_CFLAGS)
LDFLAGS += $(FTDI_LIBS)

DEPS    := mpsse_core.h

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC) $(DEPS)
	$(CC) $(CFLAGS) -o $@ $(SRC) $(LDFLAGS)

install: $(TARGET)
	install -m 755 $(TARGET) /usr/local/bin/

clean:
	$(RM) xvcd_ft2232d xvcd_ft2232d.exe
