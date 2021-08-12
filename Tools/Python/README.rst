This directory is for commonly-used python modules. Example usage:

    import os, sys

    sys.path.insert(1, os.path.expandvars('${SMING_HOME}/../Tools/Python'))

    from rjsmin import jsmin
