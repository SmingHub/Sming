#
# Build map of output files to source URLs so that edit links are created correctly
#

import os
import sys
import subprocess
import shutil

SMINGDIR = "../.."
GIT = shutil.which('git')


def gitcmd(args, **kwargs):
    kwargs['stdout'] = subprocess.PIPE
    with subprocess.Popen([GIT] + args, **kwargs) as child:
        return child.stdout.read().decode()


def urljoin(elements):
    res = ''
    for e in elements:
        if e == '':
            continue
        if res != '' and not res.endswith('/'):
            res += '/'
        res += e
    return res


def getRemotePath(srcdir):
    return gitcmd(['rev-parse', '--show-prefix'], cwd=srcdir).strip()


def getRemoteUrl(srcdir):
    return gitcmd(['config', '--get', 'remote.origin.url'], cwd=srcdir).strip()


def getRemoteCommit(srcdir):
    return gitcmd(['describe', '--always', 'HEAD'], cwd=srcdir).strip()


# Get map of directories to submodule URLs
def buildFileMap(ctx):
    # index.rst files may be generated from README.md or README.rst
    def getSourceFilename(srcdir, filename):
        if filename != 'index.rst':
            return filename
        if os.path.exists(f"{srcdir}/README.md"):
            return 'README.md'
        if os.path.exists(f"{srcdir}/README.rst"):
            return 'README.rst'
        return filename

    filemap = {}
    for dirpath, dirnames, filenames in os.walk('_inc'):
        srcpath = dirpath[5:] # skip '_inc/'
        srcdir = os.path.join(SMINGDIR, srcpath)
        # print(f"srcdir = {srcdir}", file=sys.stderr)
        remotePath = getRemotePath(srcdir)
        url = os.path.splitext(getRemoteUrl(srcdir))[0]
        commit = getRemoteCommit(srcdir)
        for f in filenames:
            s = urljoin([url, 'blob', commit, remotePath, getSourceFilename(srcdir, f)])
            filemap[f"{dirpath}/{f}"] = s

    # for k, v in filemap.items():
    #     print(f"{k}: {v}", file=sys.stderr)

    return filemap
