#!/bin/bash
set -ex # exit with nonzero exit code if anything fails

TAG=$1
if [ -z "$TAG" ]; then
	printf "Usage:\n\t$0 <tag>\n";
	exit 1;
fi

# [ Create archive of all submodules used in this release (pulled in during documentation build stage) ] 
cd "$CI_BUILD_DIR"
ALL_SUBMODULE_DIRS=$(find "$SMING_HOME" -name '.submodule' | xargs dirname | sed 's/^\(.*\)\/\(Sming\/.*\)$/\2/')
SUBMODULE_ARCHIVE=sming-submodules.tgz
tar czf $SUBMODULE_ARCHIVE $ALL_SUBMODULE_DIRS

# [ Create new draft release for this tag]
set +x
source /tmp/secrets.sh
AUTH_HEADER="Authorization: token ${RELEASE_TOKEN}"
RESPONSE=$(curl -H "Content-Type:application/json" -H "$AUTH_HEADER" \
  -d "{\"tag_name\":\"$TAG\",\"target_commitish\": \"develop\",\"name\": \"$TAG\",\"body\":\"Coming soon\",\"draft\": true,\"prerelease\": true}" \
  https://api.github.com/repos/${CI_REPO_NAME}/releases)

# Get release id
RELEASE_ID=$(echo "$RESPONSE" | jq -r .id)

upload_asset() {
  curl -H "$AUTH_HEADER" -H "Content-Type: $(file -b --mime-type $1)" --data-binary @$1 "https://uploads.github.com/repos/${CI_REPO_NAME}/releases/$RELEASE_ID/assets?name=$(basename $1)"
}

upload_asset $SUBMODULE_ARCHIVE
upload_asset sming-docs.zip 
set -x

# [ Update the choco packages ]
cd /tmp
CHOCO_REPO="github.com/slaff/chocolatey-packages.git"
git clone -q https://${CHOCO_REPO} choco
cd choco
PACKAGES_TO_CHANGE="sming sming.source"

for PACKAGE in $PACKAGES_TO_CHANGE;
do
  xmlstarlet ed --inplace -N "ns=http://schemas.microsoft.com/packaging/2010/07/nuspec.xsd" -u "/ns:package/ns:metadata/ns:version"  -v "$TAG" "packages/$PACKAGE/$PACKAGE.nuspec";
done

git config user.email "appveyor@sminghub.local"
git config user.name "appveyor"
git commit -a -m "Updated chocolatey packages to latest stable $TAG version." || 1

# Push upstream
set +x
git push https://${CHOCO_TOKEN}@${CHOCO_REPO} master
set -x
