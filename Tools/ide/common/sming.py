#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import os, sys, json, shutil, configparser, string

class Env(dict):
    """ Cache build environment variables"""

    def __init__(self):
        self.update(os.environ)
        self.updateFromConfig('config.mk')
        self.updateFromConfig('debug.mk')

    def updateFromConfig(self, filename):
        path = self['OUT_BASE'] + '/' + filename
        if not os.path.exists(path):
            return
        parser = configparser.ConfigParser()
        parser.optionxform = str # preserve case
        with open(path) as f:
            data = "[config]\n" + f.read()
        parser.read_string(data)
        self.update(parser['config'])

    def replace(self, path, name, prefix):
        value = self.get(name)
        if value is not None:
            s = fix_path(path)
            value = fix_path(value)
            if value != '' and s.startswith(value):
                return '${%s%s}%s' % (prefix, name, s[len(value):])
        return path

    def resolve(self, path):
        """Convert any embedded environment variables into real paths
        """
        tmp = str(path)
        while True:
            tmp = tmp.replace('(', '{')
            tmp = tmp.replace(')', '}')
            new_path = string.Template(tmp).safe_substitute(self)
            if new_path == tmp:
                return new_path
            tmp = new_path

    def subst_path(self, path, prefix=''):
        path = self.replace(path, 'SMING_HOME', prefix)
        path = self.replace(path, 'ESP_HOME', prefix)
        path = self.replace(path, 'IDF_PATH', prefix)
        path = self.replace(path, 'IDF_TOOLS_PATH', prefix)
        return path

    def isWsl(self):
        return self.get('WSL_ROOT', '') != ''


env = Env()


def fix_path(path):
    """Fix path so it conforms to makefile specs"""
    if path[1:3] == ':/':
        return '/' + path[0] + path[2:]
    return path

def check_path(path):
    """Fix path so it conforms to vscode specs"""
    if sys.platform == 'win32':
        if path[:1] == '/':
            return path[1:2] + ':' + path[2:]
    return path

def find_tool(name):
    if sys.platform == 'win32':
        if os.path.splitext(name)[1] != '.exe':
            name += '.exe'
    if os.path.isabs(name):
        path = name
    else:
        path = shutil.which(name)
        if path is None:
            sys.stderr.write("Warning! '%s' not found in path\n" % name)
            path = name
    if not os.path.exists(path):
        sys.stderr.write("Warning! '%s' doesn't exist\n" % path)
    return env.subst_path(path, 'env:')

def load_json(filename, must_exist=True):
    print(filename)
    if must_exist or os.path.exists(filename):
        import rjsmin
        s = open(filename, 'r').read()
        return json.loads(rjsmin.jsmin(s))
    return None

def save_json(data, filename):
    dir = os.path.dirname(filename)
    if dir != '':
        os.makedirs(dir, exist_ok=True)
    with open(filename, 'w') as f:
        json.dump(data, f, indent=4)

def load_template(name, dir = None):
    if dir == None:
        dir = os.path.dirname(__file__)
    filename = os.path.join(dir, 'template', name)
    return load_json(filename)

def find_object(data, name):
    for o in data:
        if o['name'] == name:
            return o
    return None

def get_property(data, name, default):
    if not name in data:
        data[name] = default
    return data[name]
