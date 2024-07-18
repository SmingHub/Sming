#!/bin/bash
#
# Send test notification to CI framework
#
# Don't call directly; use makefile functions in `util.mk`
#

set -e

testname="$1"
status="$2"

testfile=/tmp/$MODULE.test
mkdir -p "$(dirname "$testfile")"
curNanos=$(date +%s%N)
if [ -f "$testfile" ]; then
    prevNanos=$(<"$testfile")
    elapsedMillis=$(( (curNanos - prevNanos) / 1000000 ))
fi

case "$status" in
    "start")
        cmd=AddTest
        status=Running
        elapsedMillis=0
        ;;
    "success")
        cmd=UpdateTest
        status=Passed
        ;;
    "fail")
        cmd=UpdateTest
        status=Failed
        ;;
    *)
        echo "Invalid status: $status"
        exit 1
        ;;
esac

echo "TestNotify: $cmd $testname -Framework Sming -Filename $MODULE -Outcome $status -Duration $elapsedMillis"

echo "$curNanos" > "$testfile"
