#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100
python3 -m pip install -q --upgrade pip

if [ "$BUILD_DOCS" == "1" ]; then
	python3 -m pip install -q -r $TRAVIS_BUILD_DIR/docs/requirements.txt
fi
