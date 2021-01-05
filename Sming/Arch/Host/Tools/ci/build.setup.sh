# Host build.setup.sh

# Check coding style
make cs
DIFFS=$(git diff)
if [ "$DIFFS" != "" ]; then
  echo "!!! Coding Style issues Found!!!"
  echo "    Run: 'make cs' to fix them. "
  echo "$DIFFS"
  exit 1
fi
