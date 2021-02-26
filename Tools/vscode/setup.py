#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import os, sys, json

def fix_path(path):
    if path[1:3] == ':/':
        return '/' + path[0] + path[2:]
    return path

def env_replace(path, name, prefix):
    if name in os.environ:
        s = fix_path(path)
        value = fix_path(os.environ[name])
        if value != '' and s.startswith(value):
            return '${%s%s}%s' % (prefix, name, s[len(value):])
    else:
        print("env['%s'] not found" % name)
    return path

def env_subst_path(path, prefix=''):
    path = env_replace(path, 'SMING_HOME', prefix)
    path = env_replace(path, 'ESP_HOME', prefix)
    path = env_replace(path, 'IDF_PATH', prefix)
    path = env_replace(path, 'IDF_TOOLS_PATH', prefix)
    return path

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
    dirs = os.environ['COMPONENTS_EXTRA_INCDIR'].split()
    for i, d in enumerate(dirs):
        dirs[i] = env_subst_path(d)

    propertiesFile = '.vscode/c_cpp_properties.json'
    if os.path.exists(propertiesFile):
        properties = load_json(propertiesFile)
    else:
        properties = load_template('intellisense/properties.json')

    configurations = get_property(properties, 'configurations', [])

    arch = os.environ['SMING_ARCH']
    config = find_object(configurations, arch)

    if config is None:
        config = load_template('intellisense/configuration.json')
        config['name'] = arch
        config['defines'].append('ARCH_%s=1' % arch.upper())
        configurations.append(config)

    config['compilerPath'] = env_subst_path(os.environ['CXX'])
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

    arch = os.environ['SMING_ARCH']
    is_wsl = (os.environ['WSL_ROOT'] != '')
    config = find_object(configurations, "%s GDB" % arch)
    if not config is None:
        path = env_subst_path(os.environ['GDB'], 'env:')
        if sys.platform == 'win32':
            path = os.path.splitext(path)[0] + '.exe'
        config['miDebuggerPath'] = path
        args = "-x ${env:SMING_HOME}/Arch/%s/Components/gdbstub/gdbcmds" % arch
        if arch == 'Esp8266':
            if not is_wsl:
                args += " -b %s" % os.environ['COM_SPEED_GDB']
            config['miDebuggerServerAddress'] = os.environ['COM_PORT_GDB']
        config['miDebuggerArgs'] = args

    save_json(launch, filename)

def update_workspace():
    filename = 'sming.code-workspace'
    ws = load_json(filename, False)
    if ws is None:
        template = load_template('workspace.json')
        ws = template.copy()
        # TODO: Make any required changes to generated
        save_json(ws, filename)

def main():
    # So we can find rjsmin.py
    sys.path.append(os.path.join(os.environ['SMING_HOME'], 'Components/Storage/Tools/hwconfig'))

    update_intellisense()
    update_tasks()
    update_launch()
    update_workspace()



if __name__ == '__main__':
    main()
