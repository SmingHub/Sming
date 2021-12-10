#!/bin/bash
#
# Usage $0 <optional-directory> <optional-keyword-in-remote>
#
# Simple helper script to simplify sending spelling patches to submodules.
# The script requires Github's CLI tool "gh" to be installed.
#
# Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
#   http://github.com/SmingHub/Sming
# This file is provided under the LGPL v3 license.
# @author Nov 2021 - Slavey Karadzhov <slav@attachix.com>
#

set +e
set -x

if [ -n "$1" ]; then
    FOLDERS=$1;
else
    FOLDERS=$(git status -s . | grep -i "^ m" | cut  -d ' '  -f 3);
fi

if [ -n "$2" ]; then
  TARGET_USER=$2;
fi

CURRENTDIR=$(pwd);
for i in $FOLDERS;
do
  CHECK=${CURRENTDIR}/${i};

  if [ ! -d "$CHECK" ]; then
		continue;
	fi
	
	cd "$CHECK";

	echo "Checking $CHECK";

	if [ -n "$TARGET_USER" ]; then
    TARGET=$(git remote -v  | grep "$TARGET_USER");
    if [ -z "$TARGET" ]; then
      continue;
    fi
  fi

  if [ -z "$(git diff)" ]; then
	  continue;
	fi
		
	git difftool

  read -p 'Continue? [N/y]: ';
  if [ "${REPLY}" != "Y" ] && [ "${REPLY}" != "y" ]; then
    continue;
  fi

  # Get the original repository
  REPO_URL=$(git config --get remote.origin.url)

  # And fork it
  gh repo fork "$REPO_URL";
  PROJECT_ID=$(git config --get remote.origin.url | cut -d '/' -f 5)
  FORK_URL=git@github.com:slaff/${PROJECT_ID}
  if [[ "$FORK_URL" != *.git ]]; then
    FORK_URL=${FORK_URL}.git
  fi
  git remote add my "$FORK_URL"

  git switch -c fix/spelling
  git commit -a -m "Fixed small spelling mistakes."
  git push -u my fix/spelling
done
