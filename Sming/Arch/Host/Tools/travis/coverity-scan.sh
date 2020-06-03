#!/bin/bash

set -e

COVERITY_SCAN_PROJECT_NAME=${TRAVIS_REPO_SLUG}
COVERITY_SCAN_NOTIFICATION_EMAIL="slaff@attachix.com"
COVERITY_SCAN_BRANCH_PATTERN="ci/coverity-scan"
COVERITY_SCAN_BUILD_COMMAND="$MAKE_PARALLEL Basic_DateTime Basic_Delegates Basic_Interrupts Basic_ProgMem Basic_Serial Basic_Servo Basic_Ssl HttpServer_FirmwareUpload SMING_ARCH=Host DEBUG_VERBOSE_LEVEL=3"

# Environment check
[ -z "$COVERITY_SCAN_PROJECT_NAME" ] && echo "ERROR: COVERITY_SCAN_PROJECT_NAME must be set" && exit 1
[ -z "$COVERITY_SCAN_NOTIFICATION_EMAIL" ] && echo "ERROR: COVERITY_SCAN_NOTIFICATION_EMAIL must be set" && exit 1
[ -z "$COVERITY_SCAN_BRANCH_PATTERN" ] && echo "ERROR: COVERITY_SCAN_BRANCH_PATTERN must be set" && exit 1
[ -z "$COVERITY_SCAN_BUILD_COMMAND" ] && echo "ERROR: COVERITY_SCAN_BUILD_COMMAND must be set" && exit 1
[ -z "$COVERITY_SCAN_TOKEN" ] && echo "ERROR: COVERITY_SCAN_TOKEN must be set" && exit 1

PLATFORM=$(uname)
TOOL_ARCHIVE=/tmp/cov-analysis-${PLATFORM}.tgz
TOOL_URL=https://scan.coverity.com/download/${PLATFORM}
TOOL_BASE=/tmp/coverity-scan-analysis
UPLOAD_URL="https://scan.coverity.com/builds"
SCAN_URL="https://scan.coverity.com"

# Do not run on pull requests
if [ "${TRAVIS_PULL_REQUEST}" = "true" ]; then
  echo -e "\033[33;1mINFO: Skipping Coverity Analysis: branch is a pull request.\033[0m"
  exit 0
fi

# Verify upload is permitted
permit=true
AUTH_RES=$(curl -s --form project="$COVERITY_SCAN_PROJECT_NAME" --form token="$COVERITY_SCAN_TOKEN" $SCAN_URL/api/upload_permitted)
if [ "$AUTH_RES" = "Access denied" ]; then
  echo -e "\033[33;1mCoverity Scan API access denied. Check COVERITY_SCAN_PROJECT_NAME and COVERITY_SCAN_TOKEN.\033[0m"
  exit 1
else
  AUTH=$(echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['upload_permitted']")
  if [ "$AUTH" = "true" ]; then
    echo -e "\033[33;1mCoverity Scan analysis authorized per quota.\033[0m"
  else
    WHEN=$(echo $AUTH_RES | ruby -e "require 'rubygems'; require 'json'; puts JSON[STDIN.read]['next_upload_permitted_at']")
    echo -e "\033[33;1mOops!Coverity Scan analysis engine NOT authorized until $WHEN.\033[0m"
    permit=false
  fi
fi

if [ "$permit" = true ]; then
if [ ! -d $TOOL_BASE ]; then
  # Download Coverity Scan Analysis Tool
  if [ ! -e $TOOL_ARCHIVE ]; then
    echo -e "\033[33;1mDownloading Coverity Scan Analysis Tool...\033[0m"
    wget -nv -O $TOOL_ARCHIVE $TOOL_URL --post-data "project=$COVERITY_SCAN_PROJECT_NAME&token=$COVERITY_SCAN_TOKEN"
  fi

  # Extract Coverity Scan Analysis Tool
  echo -e "\033[33;1mExtracting Coverity Scan Analysis Tool...\033[0m"
  mkdir -p $TOOL_BASE
  CURRENT_DIR=$(pwd)
  cd $TOOL_BASE
  tar xzf $TOOL_ARCHIVE
  cd $CURRENT_DIR
fi

TOOL_DIR=$(find $TOOL_BASE -type d -name 'cov-analysis*')
export PATH=$TOOL_DIR/bin:$PATH

# Build
echo -e "\033[33;1mRunning Coverity Scan Analysis Tool...\033[0m"
COV_BUILD_OPTIONS=""
#COV_BUILD_OPTIONS="--return-emit-failures 8 --parse-error-threshold 85"
RESULTS_DIR="cov-int"
eval "${COVERITY_SCAN_BUILD_COMMAND_PREPEND}"
COVERITY_UNSUPPORTED=1 cov-build --dir $RESULTS_DIR $COV_BUILD_OPTIONS $COVERITY_SCAN_BUILD_COMMAND
cov-import-scm --dir $RESULTS_DIR --scm git --log $RESULTS_DIR/scm_log.txt 2>&1

# Upload results
echo -e "\033[33;1mTarring Coverity Scan Analysis results...\033[0m"
RESULTS_ARCHIVE=analysis-results.tgz
tar czf $RESULTS_ARCHIVE $RESULTS_DIR
SHA=$(git rev-parse --short HEAD)

echo -e "\033[33;1mUploading Coverity Scan Analysis results...\033[0m"
response=$(curl \
  --silent --write-out "\n%{http_code}\n" \
  --form project=$COVERITY_SCAN_PROJECT_NAME \
  --form token=$COVERITY_SCAN_TOKEN \
  --form email=$COVERITY_SCAN_NOTIFICATION_EMAIL \
  --form file=@$RESULTS_ARCHIVE \
  --form version=$SHA \
  --form description="Travis CI build" \
  $UPLOAD_URL)
status_code=$(echo "$response" | sed -n '$p')
if [ "$status_code" -ge "400" ]; then
  TEXT=$(echo "$response" | sed '$d')
  echo -e "\033[33;1mCoverity Scan upload failed: $TEXT. Status code: $status_code.\033[0m"
  exit 1
fi
fi
