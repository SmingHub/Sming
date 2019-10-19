#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

TAG=$1
if [ -z $TAG ]; then
	echo "Usage:\n\t$0 <tag>\n";
	exit 1;
fi

# [Update the documentation]
# On push and release readthedocs webhook should update the documentation
# See: https://buildmedia.readthedocs.org/media/pdf/docs/stable/docs.pdf Webhooks

# [ Update the choco packages ]
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