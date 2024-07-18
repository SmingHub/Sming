#!/usr/bin/env python

import os, argparse, json

def get_tool_info(soc):
    path = os.path.expandvars('${IDF_PATH}/tools/tools.json')
    with open(path, 'r') as f:  # type: ignore
        tools_info = json.load(f)

    compiler_prefix = None
    compiler_version = None
    gcc_path = gdb_path = None
    for tool in tools_info['tools']:
        if tool.get('install') != 'always':
            continue
        if soc not in tool['supported_targets']:
            continue
        desc = tool['description'].lower()
        export_path = "/".join(tool['export_paths'][0])
        tool_name = tool['name']
        tool_version = tool['versions'][0]['name']
        path = f"{tool_name}/{tool_version}/{export_path}"
        if 'gcc' in desc and not gcc_path:
            gcc_path = path
        if 'gdb' in desc and not gdb_path:
            gdb_path = path

    print(gcc_path, gdb_path)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='IDF Tools Parser')
    parser.add_argument('soc', help='Target SOC')
    args = parser.parse_args()
    get_tool_info(args.soc)
