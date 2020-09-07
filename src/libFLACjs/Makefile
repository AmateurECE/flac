###############################################################################
# NAME:             Makefile.emscripten
#
# AUTHOR:           Ethan D. Twardy <edtwardy@mtu.edu>
#
# DESCRIPTION:      Makefile for the emscripten JS library
#
# CREATED:          09/03/2020
#
# LAST EDITED:      09/07/2020
###

# How to build up to this point:
# (from root)$ ./autogen.sh && autoreconf --install && automake
# LT_MULTI_MODULE=1 emconfigure ./configure --disable-asm-optimizations
# emmake make -C src/libFLAC
# emmake make -C src/libFLAC++
# emmake make -C src/libFLAC++ -f src/libFLAC++/Makefile.emscripten

# TODO: Write a custom wasm loader and compile to standalone wasm with flags:
# -s STANDALONE_WASM --no-entry -s WASM=1
# (Change output filename to *.wasm)

CC = em++
CFLAGS = -Wall -Wextra --bind -I../../include -s MODULARIZE=1 \
       -s FILESYSTEM=0 -Os
LDFLAGS = -L../libFLAC/.libs -lFLAC-static -L../libFLAC++/.libs -lFLAC++-static
SOURCES = \
	emscripten_bindings.cpp \
	StreamDecoder.cpp

.libs/libFLAC.js: $(SOURCES)
	mkdir -p .libs
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

###############################################################################
