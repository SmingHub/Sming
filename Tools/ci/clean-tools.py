#!/usr/bin/env python

import os
import re
import shutil
import argparse

# The commented-out ARC paths below are for regular libraries with RTTI.
# These are used by default without the -fno-rtti switch so without them the cmake
# compiler check fails. Otherwise they can go.
TOOLS = r'esp32/tools/'
IDF = r'esp-idf([^/]+)/'
ARC = r'([^/]+)\.a'
FILTERS = [
    # Leave versioned directory to avoid re-installation
    rf'{TOOLS}.*esp-elf-gdb/.*/.*esp-elf-gdb/',
    rf'{TOOLS}esp32ulp-elf/.*/esp32ulp-elf/',
    rf'{TOOLS}openocd-esp32/.*/openocd-esp32/',
    # Libraries not required by Sming
    # rf'{TOOLS}.*/riscv32-esp-elf/lib/{ARC}',
    rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32i_.*',
    rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32i/',
    rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32imac_.*',
    rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32imafc_.*',
    rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32imafc/',
    # rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32imc_zicsr_zifencei/ilp32/{ARC}',
    # rf'{TOOLS}.*/riscv32-esp-elf/lib/rv32imc/ilp32/{ARC}',
    # rf'{TOOLS}.*/lib/esp32/{ARC}',
    rf'{TOOLS}.*/lib/esp32-psram',
    rf'{TOOLS}.*/lib/esp32/psram',
    # rf'{TOOLS}.*/lib/esp32s2/{ARC}',
    # rf'{TOOLS}.*/lib/esp32s3/{ARC}',
    # rf'{TOOLS}.*/xtensa-esp32-elf/lib/{ARC}',
    # rf'{TOOLS}.*/xtensa-esp32s2-elf/lib/{ARC}',
    # rf'{TOOLS}.*/xtensa-esp32s3-elf/lib/{ARC}',
    # rf'{TOOLS}.*/xtensa-esp-elf/lib/{ARC}',
    # Components, examples and archives
    rf'{IDF}docs/',
    rf'{IDF}examples/',
    rf'{IDF}components/cmock/',
    rf'{IDF}components/unity/',
    rf'{IDF}.*esp32c6.*',
    rf'{IDF}.*esp32h2.*',
    rf'{IDF}.*esp32p4.*',
    rf'{IDF}.*/tests',
]

# Python 3.8 doesn't have str.removeprefix
def removeprefix(path: str, prefix: str) -> str:
    return path[len(prefix):] if path.startswith(prefix) else path


def scan_log(logfile: str, file_list: dict):
    with open(logfile, 'r') as f:
        for path in f:
            path = path.strip()
            try:
                path, _, blocks = path.rpartition(' ')
                path, _, size = path.rpartition(' ')
                blocks = int(blocks)
                size = int(size)
                if path[1] == ':':
                    path = path[2:].replace('\\', '/')
                path = removeprefix(path, '/opt/')
                if not path.startswith('esp'):
                    continue
                existing = file_list.get(path)
                if not existing or size > existing:
                    file_list[path] = size
            except:
                print(path)
                raise

def scan_logs(log_dir) -> dict:
    file_list = {}

    for dirpath, _, filenames in os.walk(log_dir):
        for logfile in [os.path.join(dirpath, f) for f in filenames]:
            scan_log(logfile, file_list)
    return file_list


def scan_tools_dir(tools_dir) -> dict:
    file_list = {}
    def scan(root_path):
        for entry in os.scandir(root_path):
            if entry.is_dir():
                scan(entry.path)
                continue
            path = entry.path
            path = removeprefix(path, tools_dir + '/')
            if not path.startswith('esp'):
                continue
            path = path.replace('\\', '/')
            file_list[path] = entry.stat().st_size
    scan(tools_dir)
    return file_list


def scan_list(file_list: dict):
    def mbstr(size) -> str:
        mb = size / (1024*1024)
        return f'{mb:0.2f} MB'

    total_size = sum(size for _, size in file_list.items())

    print(f'{len(file_list)} files, total size {mbstr(total_size)}')

    total_size = 0
    for flt in FILTERS:
        expr = re.compile(flt)
        size = sum(size for path, size in file_list.items() if expr.match(path))
        total_size += size
        print(f'{flt}: {mbstr(size)}')

    print(f'Total size {mbstr(total_size)}')


def clean_tools_dir(tools_dir: str, do_clean: bool):
    re_filter = re.compile('|'.join(FILTERS))

    def clean_path(root_path):
        for entry in os.scandir(root_path):
            try:
                path = entry.path
                path = path.replace('\\', '/')
                path = removeprefix(path, tools_dir.replace('\\', '/') + '/')
                if entry.is_dir():
                    if re_filter.match(path + '/'):
                        print(f"rmtree {entry.path}")
                        if do_clean:
                            shutil.rmtree(entry.path)
                    else:
                        clean_path(entry.path)
                elif re_filter.match(path):
                    print(f"rm {entry.path}")
                    if do_clean:
                        os.unlink(entry.path)
            except Exception as e:
                print(f'{repr(e)}')

    clean_path(tools_dir)


def main():
    parser = argparse.ArgumentParser(description='Sming CI tool cleaner')
    parser.add_argument('action', choices=['scan', 'clean'], help='Action to perform')
    parser.add_argument('--logdir', help='Scan downloaded log files instead of actual path')
    parser.add_argument('--delete', action="store_true", help='Required flag to actually delete')

    args = parser.parse_args()

    tools_dir = os.path.dirname(os.environ['IDF_PATH'])

    print(f'Action: {args.action} "{tools_dir}"')

    if args.action == 'scan':
        if args.logdir:
            file_list = scan_logs(args.logdir)
        else:
            file_list = scan_tools_dir(tools_dir)
        scan_list(file_list)
    elif args.action == 'clean':
        if not os.path.exists(tools_dir):
            print(f'"{tools_dir}" not found, skipping.')
            return
        clean_tools_dir(tools_dir, args.delete)
        if args.delete:
            print("OK, items cleaned.")
        else:
            print("Dry run, nothing deleted.")

if __name__ == '__main__':
    main()
