#!/bin/bash
set -e # exit with nonzero exit code if anything fails

TAG=$1
if [ -z $TAG ]; then
	echo "Usage:\n\t$0 <tag>\n";
	exit 1;
fi


# Get information about the release
# TODO: ...

# Update documentation
cd $SMING_HOME
make docs
cd ..

git fetch origin gh-pages:gh-pages
git checkout gh-pages

DOCS_DIR=$SMING_HOME/../api

rm -rf $DOCS_DIR
cp -r $SMING_HOME/../docs/api/sming/ $DOCS_DIR
git add -A $DOCS_DIR 
git commit -m "Updated the API docs to version $TAG." || 1

git push https://${SMING_TOKEN}@github.com/${TRAVIS_REPO_SLUG}.git gh-pages
