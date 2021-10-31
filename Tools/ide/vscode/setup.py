#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import os, sys

appPath = os.path.dirname(os.path.realpath(__file__))
libPath = appPath + '/../common/'
sys.path.append(libPath)

from sming import check_path, env, find_object, find_tool, get_property, load_json, load_template, save_json

def update_intellisense():
    dirs = []
    for d in env['COMPONENTS_EXTRA_INCDIR'].split():
        d = check_path(d)
        if os.path.exists(d):
            dirs += [check_path(env.subst_path(d))]

    propertiesFile = '.vscode/c_cpp_properties.json'
    if os.path.exists(propertiesFile):
        properties = load_json(propertiesFile)
    else:
        properties = load_template('intellisense/properties.json', appPath)

    env.update(get_property(properties, 'env', {}))
    configurations = get_property(properties, 'configurations', [])

    config = find_object(configurations, env['SMING_ARCH'])

    if config is None:
        config = load_template('intellisense/configuration.json', appPath)
        config['name'] = env['SMING_ARCH']
        configurations.append(config)

    args = env['APP_CFLAGS'].split() + env['GLOBAL_CFLAGS'].split()
    args = [env.resolve(v) for v in args]
    config['compilerArgs'] = [v for v in args if v != '']

    config['compilerPath'] = find_tool(env['CXX'])
    config['includePath'] = dirs

    save_json(properties, propertiesFile)


def update_tasks():
    filename = '.vscode/tasks.json'
    launch = load_json(filename, False)
    if launch is None:
        template = load_template('tasks.json', appPath)
        launch = template.copy()
        # TODO: Make any changes as required
        save_json(launch, filename)

def update_launch():
    filename = '.vscode/launch.json'
    launch = load_json(filename, False)
    template = load_template('launch.json', appPath)
    if launch is None:
        launch = template.copy()
    configurations = get_property(launch, 'configurations', [])
    config_name = "%s GDB" % env['SMING_ARCH']
    config = find_object(configurations, config_name)
    template_config = find_object(template['configurations'], config_name)
    if template_config is None:
        print("Warning: Template launch configuration '%s' not found" % config_name)
    elif not config is None:
        configurations.remove(config)
        config = template_config.copy()
        configurations.append(config)

    if config is None:
        return

    config['miDebuggerPath'] = find_tool(env['GDB'])
    dbgargs = "-x ${env:SMING_HOME}/Arch/%s/Components/gdbstub/gdbcmds" % env['SMING_ARCH']
    if env['SMING_ARCH'] == 'Host':
        args = []
        args += env['CLI_TARGET_OPTIONS'].split()
        args += ["--"]
        args += env['HOST_PARAMETERS'].split()
        config['args'] = args
    else:
        if not env.isWsl():
            dbgargs += " -b %s" % env.resolve('${COM_SPEED_GDB}')
        config['miDebuggerServerAddress'] = env.resolve('${COM_PORT_GDB}')
    config['miDebuggerArgs'] = dbgargs
    config['program'] = "${workspaceFolder}/" + env.resolve('${TARGET_OUT_0}')

    save_json(launch, filename)

def update_workspace():
    filename = 'sming.code-workspace'
    ws = load_json(filename, False)
    template = load_template('workspace.json', appPath)
    if ws is None:
        ws = template.copy()
    schemas = ws['settings']['json.schemas'] = []
    # ws['settings']['json.schemas']
    for schema in template['settings']['json.schemas']:
        schema['url'] = env.resolve(schema['url'])
        schemas += [schema]
    save_json(ws, filename)

def main():
    if not env['SMING_HOME'] or not env['SMING_ARCH']:
        sys.exit(1)

    # So we can find rjsmin.py
    sys.path.append(os.path.join(env['SMING_HOME'], '../Tools/Python'))

    update_intellisense()
    update_tasks()
    update_launch()
    update_workspace()



if __name__ == '__main__':
    main()
