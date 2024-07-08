#!/usr/bin/env python

import os
import re
import shutil
import argparse

# Mandatory environment variables
IDF_PATH = os.environ['IDF_PATH']
IDF_TOOLS_PATH = os.environ['IDF_TOOLS_PATH']

# The commented-out ARC paths below are for regular libraries with RTTI.
# These are used by default without the -fno-rtti switch so without them the cmake
# compiler check fails. Otherwise they can go.
TOOLS = r'esp32/tools/'
IDF = r'esp-idf([^/]+)/'

ARC = r'([^/]+)\.a'

# These filters are matched from the **start** of the path so there's an implicit .* at the end
FILTERS = {
    IDF_TOOLS_PATH: [
        # Leave versioned directory to avoid re-installation
        r'.*esp-elf-gdb/.*/.*esp-elf-gdb/',
        r'esp32ulp-elf/.*/esp32ulp-elf/',
        r'openocd-esp32/.*/openocd-esp32/',
        # Libraries not required by Sming
        # r'.*/riscv32-esp-elf/lib/{ARC}',
        r'.*/riscv32-esp-elf/lib/rv32i_.*',
        r'.*/riscv32-esp-elf/lib/rv32i/',
        r'.*/riscv32-esp-elf/lib/rv32imac_.*',
        r'.*/riscv32-esp-elf/lib/rv32imafc_.*',
        r'.*/riscv32-esp-elf/lib/rv32imafc/',
        # r'.*/riscv32-esp-elf/lib/rv32imc_zicsr_zifencei/ilp32/{ARC}',
        # r'.*/riscv32-esp-elf/lib/rv32imc/ilp32/{ARC}',
        # r'.*/lib/esp32/{ARC}',
        r'.*/lib/esp32(-|/)psram',
        # r'.*/lib/esp32s2/{ARC}',
        # r'.*/lib/esp32s3/{ARC}',
        # r'.*/xtensa-esp32-elf/lib/{ARC}',
        # r'.*/xtensa-esp32s2-elf/lib/{ARC}',
        # r'.*/xtensa-esp32s3-elf/lib/{ARC}',
        # r'.*/xtensa-esp-elf/lib/{ARC}',
    ],
    IDF_PATH: [
        # Components, examples and archives
        r'docs/',
        r'.*/doc',
        r'examples/',
        r'components/asio/',
        r'components/cmock/',
        r'components/openthread/openthread/third_party',
        r'components/unity/',
        r'components/.*esp32c6.*',
        r'components/.*esp32h2.*',
        r'components/.*esp32p4.*',
        r'components/.*/test',
        r'components/.*/fuzz',
        r'components/expat/expat/testdata',
        r'components/libsodium',
        r'components/nghttp/nghttp2/third-party/mruby',
        r'components/nghttp/nghttp2/third-party',
        r'components/tinyusb/',
        r'components/.*/win32',
        r'tools/esp_app_trace',
        r'tools/test',
        r'tools/ci',
    ]
}

def fix_path(path: str) -> str:
    return path[2:].replace('\\', '/') if path[1] == ':' else path


def scan_log(logfile: str, file_list: dict):
    with open(logfile, 'r', encoding='utf8') as f:
        for path in f:
            path = path.strip()
            try:
                path, _, blocks = path.rpartition(' ')
                path, _, size = path.rpartition(' ')
                blocks = int(blocks)
                size = int(size)
                path = fix_path(path)
                if '/esp' not in path:
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


def scan_tree(start_path: str, file_list: dict):
    def scan(root_path):
        for entry in os.scandir(root_path):
            if entry.is_dir():
                scan(entry.path)
                continue
            path = fix_path(entry.path)
            file_list[path] = entry.stat().st_size
    scan(start_path)
    return file_list


def scan_list(file_list: dict):
    def mbstr(size) -> str:
        mb = size / (1024*1024)
        return f'{mb:0.2f} MB'

    total_size = sum(size for _, size in file_list.items())

    print(f'{len(file_list)} files, total size {mbstr(total_size)}')

    def match(start_path: str, filters: list):
        start_path = fix_path(start_path)
        matched_size = 0
        for flt in filters:
            flt = rf'{start_path}/{flt}'
            expr = re.compile(flt, flags=re.IGNORECASE)
            size = sum(size for path, size in file_list.items() if expr.match(path))
            matched_size += size
            print(f'{flt}: {mbstr(size)}')
        return matched_size

    total_size = sum(match(*item) for item in FILTERS.items())

    print(f'Total size {mbstr(total_size)}')


def clean_tree(start_path: str, filters: list, do_clean: bool):
    if not os.path.exists(start_path):
        print(f'"{start_path}" not found, skipping.')
        return

    re_filter = re.compile('|'.join(rf'{fix_path(start_path)}/{f}' for f in filters), flags=re.IGNORECASE)

    def clean_path(root_path):
        for entry in os.scandir(root_path):
            try:
                path = fix_path(entry.path)
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

    clean_path(start_path)


def main():
    parser = argparse.ArgumentParser(description='Sming CI tool cleaner')
    parser.add_argument('action', choices=['scan', 'clean'], help='Action to perform')
    parser.add_argument('--logdir', help='Scan downloaded log files instead of actual path')
    parser.add_argument('--delete', action="store_true", help='Required flag to actually delete')

    args = parser.parse_args()

    print(f'Action: {args.action}, IDF_PATH="{IDF_PATH}", IDF_TOOLS_PATH="{IDF_TOOLS_PATH}"')

    if args.action == 'scan':
        if args.logdir:
            file_list = scan_logs(args.logdir)
        else:
            file_list = {}
            scan_tree(IDF_PATH, file_list)
            scan_tree(IDF_TOOLS_PATH, file_list)
        scan_list(file_list)
        return

    if args.action == 'clean':
        for path, filters in FILTERS.items():
            clean_tree(path, filters, args.delete)
        if args.delete:
            print("** Cleaning finished.")
        else:
            print("** Dry run, nothing deleted.")

if __name__ == '__main__':
    main()
