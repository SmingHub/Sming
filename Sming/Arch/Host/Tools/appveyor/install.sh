sudo apt-get install -y clang-format-6.0 gcc-multilib g++-multilib \
  python3-sphinx python3-pip python3-setuptools python3-cairocffi \
  doxygen graphviz-dev xmlstarlet jq
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-6.0 100
python3 -m pip install --upgrade pip
python3 -m pip install -r $APPVEYOR_BUILD_FOLDER/docs/requirements.txt
