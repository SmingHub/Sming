#!/usr/bin/env bash
#
# Update and patch llhttp source code from repository
#

set -e

if [ -z "$1" ]; then
    echo "Usage: $0 [git path]"
    echo "   - Clone https://github.com/nodejs/llhttp.git to working directory"
    echo "   - Build as per instructions"
    echo "   - Run this script to update Sming source"
    exit 1
fi

srcdir="$1"

srcfiles=(\
    build/c/llhttp.c \
    build/llhttp.h \
    src/native/api.c \
    src/native/http.c \
)

for f in ${srcfiles[@]}; do
    cp "$srcdir/$f" src/
done

# Using SED on generated code as changes are more likely

sed -i -E 's/state->reason = "(.+)"/state->reason = _F("\1")/' src/llhttp.c
sed -i -E 's/static uint8_t lookup_table\[\] =/static const uint8_t lookup_table\[\] PROGMEM =/' src/llhttp.c
sed -i -E 's/lookup_table\[\(uint8_t\) \*p\]/pgm_read_byte\(\&lookup_table\[\(uint8_t\) \*p\]\)/' src/llhttp.c

sed -i -E '/^#include <stdint.h>$/a#include <FakePgmSpace.h>' src/llhttp.h
sed -i -E '/^enum llhttp_method \{$/,/^};$/s/HTTP_/HTTP_METHOD_/' src/llhttp.h

# Regular patch on API code

git apply -v llhttp.patch --ignore-whitespace --whitespace=nowarn
