#!/usr/bin/env python3
#
# Tool to scan files for extended unicode characters, such as smart quotes
#
# Should be pretty simple. All files should be UTF-8 encoded so text gets converted to unicode by python.
# We then check for characters outside the normal ASCII range and report them.
#
# Usage: In vscode, open an integrated terminal and run this tool:
#
#   sming$ python docs/Tools/uniscan.py .
#
# You'll then get a screenful of links to browse through.
#
# See https://github.com/ProfJordan/smart-quotes-normalizer for a handy tool to replace
# smart quotes within VSCode.
#

import argparse
import os

EXT_TO_SCAN = ['.c', '.cpp', '.h', '.hpp', '.s', '.rst', '.md', '.txt']

# Any directory paths ending with these strings is ignored, case-insensitive
PATHS_TO_EXCLUDE = [
    'Sming/Arch/Esp8266/Components/esp8266/sdk',
    'Sming/Arch/Esp8266/Components/esp-open-lwip/esp-open-lwip',
    'Sming/Arch/Esp8266/Components/lwip2/lwip2',
    '/lwip/lwip',
    'Sming/Arch/Rp2040/Components/rp2040/pico-sdk',
    '/googletest',
    'TFT_S1D13781/fonts',
    '/Graphics/resource/fonts/Linux',
    '/out',
    '/HostTests/resource',
]

# Any file paths ending with these strings is ignored, case-insensitive
FILES_TO_EXCLUDE = [
    '/LICENSE.txt',
    '/locale/zh-CN.h',
    '/jvmTest/resources/utf8_sample.txt',
]

def scan_file(filename: str):
    def report(msg: str):
        print(f'{filename}:{idx+1}: {msg}')
    def check_string(s: str):
        OK_CHARS = [
            '\t', '\r', '\n',
            '°',
            '©',
            '—',
            'ε',
            'Ω',
            '–', # en-dash U+2013
        ]
        SMART_QUOTES = [
            '‘',  # U+2018
            '’',  # U+2019
            '“',  # U+201c left
            '”',  # U+201d right
        ]
        for c in s:
            if ' ' <= c <= chr(255):
                continue
            if c in OK_CHARS:
                continue
            if c in SMART_QUOTES:
                report(f'Smart quote {c} #{ord(c):x}')
                break
            report(f'Suspect character {c} #{ord(c):x}')
            break

    with open(filename, 'rb') as f:
        for idx, line in enumerate(f):
            try:
                s = line.decode('utf-8')
                check_string(s)
            except UnicodeDecodeError as e:
                report(repr(e))
                break


def is_excluded(path: str, exclusions: list):
    path = path.lower()
    for s in exclusions:
        if path.endswith(s.lower()):
            return True
    return False


def scan_tree(path: str):
    if is_excluded(path, PATHS_TO_EXCLUDE):
        return
    for entry in os.scandir(path):
        if entry.is_dir():
            scan_tree(entry.path)
            continue
        if is_excluded(entry.path, FILES_TO_EXCLUDE):
            continue
        ext = os.path.splitext(entry.name)[1]
        if ext not in EXT_TO_SCAN:
            continue
        scan_file(entry.path)


def main():
    parser = argparse.ArgumentParser(description='Unicode scanner')
    parser.add_argument('path', help='Path to start scanning')

    args = parser.parse_args()

    scan_tree(args.path)


if __name__ == '__main__':
    main()
