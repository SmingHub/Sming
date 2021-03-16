#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import os, sys, json, shutil

class Env:
    """ Cache required environment variables"""

    def __init__(self):
        self.SMING_HOME = os.environ['SMING_HOME']
        self.SMING_ARCH = os.environ['SMING_ARCH']
        self.WSL_ROOT = os.environ.get('WSL_ROOT', '')
        vars = []
        if self.SMING_ARCH == 'Esp8266':
            vars += ['ESP_HOME']
        if self.SMING_ARCH == 'Esp32':
            vars += ['IDF_PATH', 'IDF_TOOLS_PATH']
        for name in vars:
            value = os.environ.get(name, None)
            if not value:
                print("Warning: env['%s'] not found" % name)
            setattr(self, name, value)

    def replace(self, path, name, prefix):
        value = getattr(self, name, None)
        if value is not None:
            s = fix_path(path)
            value = fix_path(value)
            if value != '' and s.startswith(value):
                return '${%s%s}%s' % (prefix, name, s[len(value):])
        return path

    def resolve(self, path):
        """Convert any embedded environment variables into real paths
        """
        return os.path.expandvars(path)

    def subst_path(self, path, prefix=''):
        path = self.replace(path, 'SMING_HOME', prefix)
        path = self.replace(path, 'ESP_HOME', prefix)
        path = self.replace(path, 'IDF_PATH', prefix)
        path = self.replace(path, 'IDF_TOOLS_PATH', prefix)
        return path

    def isWsl(self):
        return self.WSL_ROOT != ''

    def update(self, env):
        env['SMING_HOME'] = self.SMING_HOME
        if self.SMING_ARCH == 'Esp8266':
            env['ESP_HOME'] = self.ESP_HOME
        if self.SMING_ARCH == 'Esp32':
            env['IDF_PATH'] = self.IDF_PATH
            env['IDF_TOOLS_PATH'] = self.IDF_PATH


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

def load_template(name):
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

def update_intellisense():
    dirs = []
    for d in os.environ['COMPONENTS_EXTRA_INCDIR'].split():
        if os.path.exists(d):
            dirs += [check_path(env.subst_path(d))]

    propertiesFile = '.vscode/c_cpp_properties.json'
    if os.path.exists(propertiesFile):
        properties = load_json(propertiesFile)
    else:
        properties = load_template('intellisense/properties.json')

    env.update(get_property(properties, 'env', {}))
    configurations = get_property(properties, 'configurations', [])

    config = find_object(configurations, env.SMING_ARCH)

    if config is None:
        config = load_template('intellisense/configuration.json')
        config['name'] = env.SMING_ARCH
        config['defines'].append('ARCH_%s=1' % env.SMING_ARCH.upper())
        configurations.append(config)

    config['compilerPath'] = find_tool(os.environ['CXX'])
    config['includePath'] = dirs

    save_json(properties, propertiesFile)


def update_tasks():
    filename = '.vscode/tasks.json'
    launch = load_json(filename, False)
    if launch is None:
        template = load_template('tasks.json')
        launch = template.copy()
        # TODO: Make any changes as required
        save_json(launch, filename)

def update_launch():
    filename = '.vscode/launch.json'
    launch = load_json(filename, False)
    template = load_template('launch.json')
    if launch is None:
        launch = template.copy()
    configurations = get_property(launch, 'configurations', [])
    for template_config in template['configurations']:
        config = find_object(configurations, template_config['name'])
        if not config is None:
            configurations.remove(config)
        config = template_config.copy()
        configurations.append(config)

    config = find_object(configurations, "%s GDB" % env.SMING_ARCH)
    if not config is None:
        config['miDebuggerPath'] = find_tool(os.environ['GDB'])
        dbgargs = "-x ${env:SMING_HOME}/Arch/%s/Components/gdbstub/gdbcmds" % env.SMING_ARCH
        if env.SMING_ARCH == 'Esp8266':
            if not env.isWsl():
                dbgargs += " -b %s" % os.environ['COM_SPEED_GDB']
            config['miDebuggerServerAddress'] = os.environ['COM_PORT_GDB']
        elif env.SMING_ARCH == 'Host':
            args = []
            args += os.environ['CLI_TARGET_OPTIONS'].split()
            args += ["--pause"]
            args += ["--"]
            args += os.environ['HOST_PARAMETERS'].split()
            config['args'] = args
        config['miDebuggerArgs'] = dbgargs

    save_json(launch, filename)

def update_workspace():
    filename = 'sming.code-workspace'
    ws = load_json(filename, False)
    template = load_template('workspace.json')
    if ws is None:
        ws = template.copy()
    schemas = ws['settings']['json.schemas'] = []
    # ws['settings']['json.schemas']
    for schema in template['settings']['json.schemas']:
        schema['url'] = env.resolve(schema['url'])
        schemas += [schema]
    save_json(ws, filename)

def main():
    if not env.SMING_HOME or not env.SMING_ARCH:
        sys.exit(1)

    # So we can find rjsmin.py
    sys.path.append(os.path.join(env.SMING_HOME, 'Components/Storage/Tools/hwconfig'))

    update_intellisense()
    update_tasks()
    update_launch()
    update_workspace()



if __name__ == '__main__':
    main()
