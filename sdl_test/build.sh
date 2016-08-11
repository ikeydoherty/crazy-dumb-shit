#!/bin/bash

# Because I CBF to write autotools right now

# i.e. AM_CFLAGS
CFLAGS="$CFLAGS -fstack-protector -Wall -pedantic \
-Wstrict-prototypes -Wundef -fno-common \
-Werror-implicit-function-declaration \
-Wformat -Wformat-security -Werror=format-security \
-Wconversion -Wunused-variable -Wunreachable-code \
-Wall -W -D_FORTIFY_SOURCE=2 -std=c11"

# PKG_CHECK_MODULES
PKG_FLAGS="`pkg-config --cflags --libs sdl2` `pkg-config --cflags --libs SDL2_ttf`"

# Avoid unnecessary linkage, leverage the Solus toolchain
export LD_AS_NEEDED=1

# Go build it.
gcc $CFLAGS $LDFLAGS sdl_basic.c $PKG_FLAGS-o sdl_basic
