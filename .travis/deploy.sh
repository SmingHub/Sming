#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

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
git commit -m "Updated the API docs to version $TAG." || true

git push https://${SMING_TOKEN}@github.com/${TRAVIS_REPO_SLUG}.git gh-pages

# Update the Choco packages
cd /tmp
git clone https://github.com/slaff/chocolatey-packages.git
cd chocolatey-packages
FILES_TO_CHANGE="packages/sming/sming.nuspec packages/sming.upgrade/sming.upgrade.nuspec packages/sming.examples/sming.examples.nuspec packages/sming.core/sming.core.nuspec"

for FILE in $FILES_TO_CHANGE;
do
     xmlstarlet ed --inplace -N "ns=http://schemas.microsoft.com/packaging/2010/07/nuspec.xsd" -u "/ns:package/ns:metadata/ns:version"  -v "$TAG" $FILE;
done

git commit -a -m "Updated chocolatey packages to latest stable $TAG version." || 1
git push https://${SMING_TOKEN}@github.com/slaff/chocolatey-packages.git master