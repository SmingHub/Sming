# Documentation build tools

PACKAGES=(\
    doxygen \
    python3-sphinx \
    python3-cairocffi \
    )

case $DIST in
    debian)
        PACKAGES+=(\
            graphviz-dev \
            )
        ;;

    fedora)
        PACKAGES+=(\
            python-wheel \
            graphviz-devel \
            )
        ;;
esac

$PKG_INSTALL "${PACKAGES[@]}"

python3 -m pip install -r "$SMING_HOME/../docs/requirements.txt"
