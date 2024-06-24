#!/bin/bash
#
# Combines firmware BLOBs into a single file with headers indicating size.
# No padding is necessary.
# A simple chunk header is written which avoids need to hard-code sizes into driver.
#

set -e

write_chunk() {
    sz=$(printf "%08x" $(wc -c "$1" | awk '{print $$1}'))
    # Output 8-byte header containing chunk tag plus length in little-endian format
    printf "CHNK\x${sz:6:2}\x${sz:4:2}\x${sz:2:2}\x${sz:0:2}" >> $2
    # Append chunk data
    cat "$1" >> $2
}

OUTFILE="${1/.gz/}"
rm -f "$OUTFILE"
write_chunk "43439A0-7.95.49.00.bin" "$OUTFILE"
write_chunk "clm_blob.bin" "$OUTFILE"
gzip --best "$OUTFILE"
