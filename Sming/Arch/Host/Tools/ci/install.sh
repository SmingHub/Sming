# Host install.sh

sudo apt-get install -y clang-format-6.0 \
  python3-sphinx python3-setuptools python3-cairocffi \
  doxygen graphviz-dev xmlstarlet jq
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100

python -m pip install -r $SMING_HOME/../docs/requirements.txt
