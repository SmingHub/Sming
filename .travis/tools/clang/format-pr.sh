#!/bin/bash

# Script taken from here: https://github.com/citra-emu/citra
# and is distributed under the same license as the original script

# Default clang-format points to default 3.5 version one
CLANG_FORMAT=/usr/bin/clang-format
$CLANG_FORMAT --version

cd $SMING_HOME/..

# Get list of modified files
files_to_lint="$(git diff --name-only --diff-filter=ACMRTUXB $TRAVIS_COMMIT_RANGE | grep '^[^.]*[.]\(c\|cpp\|h\)$' | grep -e Sming/Core -e Sming/Platform -e Sming/Arch/.+/Core -e Sming/Arch/.+/Platform -e samples || true)"

for f in $files_to_lint; do

    d=$(diff -u "$f" <($CLANG_FORMAT "$f") || true)
    if ! [ -z "$d" ]; then
        echo "!!! $f not compliant to coding style, here is the fix:"
        echo "$d"
        fail=1
    fi
done

if [ "$fail" = 1 ]; then
    exit 1
fi