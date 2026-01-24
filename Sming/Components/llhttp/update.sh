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
    LICENSE \
    build/c/llhttp.c \
    build/llhttp.h \
    src/native/api.c \
    src/native/http.c \
)

for f in ${srcfiles[@]}; do
    cp "$srcdir/$f" src/
done

# Using SED on generated code as changes are more likely

SED="sed -i -E"

$SED '/^#include <stdint.h>$/ a\#include <sys/pgmspace.h>\n\n#define LLHTTP_REASON_STR\(x\) NULL' src/llhttp.h

# Rename llhttp_method enumerated values
$SED '/^enum llhttp_method \{$/, /^};$/ s/HTTP_/HTTP_METHOD_/' src/llhttp.h
$SED 's/(parser->method == HTTP_)/\1METHOD_/' src/http.c

# Rename llhttp_status enumerated values
$SED '/^enum llhttp_status \{$/, /^};$/ s/HTTP_STATUS_/HTTP_STATUS_CODE_/' src/llhttp.h

# Rename llhttp_errno enumerated values
$SED '/^enum llhttp_errno \{$/, /^};$/ s/HPE_/HTTP_ERRNO_/' src/llhttp.h
$SED 's/HPE_/HTTP_ERRNO_/' src/api.c

# Don't output reason strings to RAM. Use macro so these can be enabled if required.
$SED 's/(state->reason = )"(.+)"/\1LLHTTP_REASON_STR("\2")/' src/llhttp.c
$SED 's/("Span callback error in " #NAME)/LLHTTP_REASON_STR(\1)/' src/api.c

# Put lookup tables in flash
$SED 's/static (uint8_t lookup_table\[\]) =/static const \1 PROGMEM =/' src/llhttp.c
$SED 's/(lookup_table\[\(uint8_t\) \*p\])/pgm_read_byte\(\&\1)/' src/llhttp.c

# Put parser blobs in flash
$SED 's/(llparse_blob(.+)\[\])/\1 PROGMEM/' src/llhttp.c
$SED 's/(current ==) (seq\[index\])/\1 pgm_read_byte\(\&\2\)/' src/llhttp.c

# Remove unused functions which may consume RAM if linked
$SED '/llhttp_errno_name/,/^}/d' src/api.c
$SED '/llhttp_method_name/,/^}/ s/HTTP_##NAME: return #STRING/HTTP_METHOD_##NAME: return PSTR(#STRING)/' src/api.c
$SED '/llhttp_status_name/,/^}/d' src/api.c
