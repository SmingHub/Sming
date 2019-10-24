#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

TAG=$1
if [ -z $TAG ]; then
	echo "Usage:\n\t$0 <tag>\n";
	exit 1;
fi

export SMING_HOME=$TRAVIS_BUILD_DIR/Sming

# [ Create new draft release for this tag]
AUTH_HEADER="Authorization: token ${RELEASE_TOKEN}"
RESPONSE=$(curl -H "Content-Type:application/json" -H "$AUTH_HEADER" \
  -d "{\"tag_name\":\"$TAG\",\"target_commitish\": \"develop\",\"name\": \"$TAG\",\"body\":\"Coming soon\",\"draft\": true,\"prerelease\": true}" \
  https://api.github.com/repos/SmingHub/Sming/releases)

# Get release id
$ID=$(echo $RESPONSE | jq -r .id)

# [Get all submodules used in this release, pack them and add the archive to the release artifacts]
cd $SMING_HOME
make submodules
ALL_SUBMODULE_DIRS=$(find $SMING_HOME -name '.submodule' | xargs dirname)
FILE=/tmp/sming-submodules.tgz
tar cvzf $FILE $ALL_SUBMODULE_DIRS

curl -H "$AUTH_HEADER" -H "Content-Type: $(file -b --mime-type $FILE)" --data-binary @$FILE "https://uploads.github.com/repos/SmingHub/Sming/releases/$ID/assets?name=$(basename $FILE)"

# [Update the documentation]
# On push and release readthedocs webhook should update the documentation automatically.
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
