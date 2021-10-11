#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import os, sys
from shutil import copyfile
from pathlib import Path

appPath = os.path.dirname(os.path.realpath(__file__))
libPath = appPath + '/../common/'
print(libPath)
sys.path.append(libPath)

from sming import *

def update_project_files():
    projectName = os.path.basename(os.getcwd())
    content = Path(appPath + '/templates/.project').read_text()
    content = content % { 'PROJECT_NAME': projectName }
    with open(os.getcwd() + '/.project', 'w') as f:
        f.write(content)
    copyfile(appPath + '/templates/.cproject', os.getcwd() + '/.cproject')

def main():
    if not env['SMING_HOME'] or not env['SMING_ARCH']:
        sys.exit(1)
    update_project_files()


if __name__ == '__main__':
    main()
