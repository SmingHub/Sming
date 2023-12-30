#!/usr/bin/env python

import os, argparse, json

def get_tool_info(soc):
    path = os.path.expandvars('${IDF_PATH}/tools/tools.json')
    with open(path, 'r') as f:  # type: ignore
        tools_info = json.load(f)

    compiler_prefix = None
    compiler_version = None
    for tool in tools_info['tools']:
        if tool.get('install') != 'always':
            continue
        if soc not in tool['supported_targets']:
            continue
        desc = tool['description']
        if not desc.startswith('Toolchain'):
            continue
        tool_name = tool['name']
        tool_version = tool['versions'][0]['name']
        break
    print(f"{tool_name} {tool_version}")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='IDF Tools Parser')
    parser.add_argument('soc', help='Target SOC')
    args = parser.parse_args()
    get_tool_info(args.soc)
