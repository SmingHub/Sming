#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

# Check coding style
make cs
DIFFS=$(git diff)
if [ "$DIFFS" != "" ]; then
  echo "!!! Coding Style issues Found!!!"
  echo "    Run: 'make cs' to fix them. "
  echo "$DIFFS"
  exit 1
fi

# Check if we could run static code analysis
CHECK_SCA=0
if [[ $TRAVIS_TAG != "" || ( $TRAVIS_COMMIT_MESSAGE == *"[scan:coverity]"*  && $TRAVIS_PULL_REQUEST != "true" ) ]]; then
  CHECK_SCA=1
fi
