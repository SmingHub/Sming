#!/usr/bin/env python

import os, sys, argparse, json

def get_tool_info(soc):
    path = os.path.expandvars('${IDF_PATH}/tools/tools.json')
    with open(path, 'r') as f:  # type: ignore
        tools_info = json.load(f)

    compiler_prefix = None
    compiler_version = None
    for tool in tools_info['tools']:
        if soc in tool['supported_targets']:
            desc = tool['description']
            if desc.startswith('Toolchain'):
                compiler_prefix = tool['name']
                compiler_version = tool['versions'][0]['name']
                break
    print(f"{compiler_prefix} {compiler_version}")

    # ESP32_COMPILER_PREFIX=compiler_prefix
    # IDF_TARGET_ARCH_RISCV=is_riscv




if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='IDF Tools Parser')
    parser.add_argument('soc', help='Target SOC')
    args = parser.parse_args()
    get_tool_info(args.soc)
