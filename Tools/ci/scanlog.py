#!/usr/bin/env python
#
# Python application to scan CI build logs and extract useful information
#
# Github action logs must be in raw (text) format for processing.
#
# Use cases:
#
#   Generate table of esp8266 memory usage
#       Take two log files (cut & paste usually)
#       Run scanlog on both
#       Use kdiff3 to compare
#

import argparse
import os
from enum import Enum


class State(Enum):
    searching = 1
    building = 2
    linking = 3


class Table:
    COL_SEP = '  '

    def __init__(self, name: str):
        self.name = name
        self.headings = []
        self.rows = []
        self.col_widths = []

    def append(self, row_data: dict):
        for k in row_data.keys():
            if k in self.headings:
                continue
            self.headings.append(k)
            self.col_widths.append(len(k))
        row = ['' for x in self.headings]
        for k, v in row_data.items():
            i = self.headings.index(k)
            row[i] = v
            self.col_widths[i] = max(self.col_widths[i], len(v))
        self.rows.append(row)

    def _format_values(self, values: list):
        return self.COL_SEP.join(str(v).ljust(self.col_widths[i]) for i, v in enumerate(values))

    def format_headings(self):
        return self._format_values(self.headings)

    def format_separator(self):
        return self.COL_SEP.join(''.ljust(w, '-') for w in self.col_widths)

    def format_row(self, row: int | list):
        if isinstance(row, int):
            row = self.rows[row]
        return self._format_values(row)

    def find_row(self, name: str):
        return next((i for i, row in enumerate(self.rows) if row[0] == name), -1)

    def __iter__(self):
        return TableFormatter(self)


class TableFormatter:
    def __init__(self, table: Table):
        self.table = table
        self.row_index = -2

    def __next__(self):
        table = self.table
        idx = self.row_index
        if idx == -2:
            self.row_index = -1
            return table.format_headings()
        if idx == -1:
            self.row_index = 0
            return table.format_separator()
        if idx < len(table.rows):
            self.row_index = idx + 1
            return table.format_row(idx)
        raise StopIteration


def scan_log(filename: str):
    state = State.searching
    table = Table(os.path.basename(filename))
    target = None
    row = None
    log = open(filename, 'rb')
    for line in log:
        line = line.decode('utf-8-sig')
        dtstr, _, line = line.strip().partition(' ')
        if not dtstr:
            continue
        _, sep, c = line.partition('** Building ')
        if sep:
            target, _, _ = c.partition(' ')
            target = target.removeprefix('/home/runner/projects/')
            row = {
                'target': target
            }
            state = State.building
            continue
        if state == State.building:
            if line.startswith(f'{os.path.basename(target)}: Linking'):
                state = State.linking
                continue
        if state == State.linking:
            if 'Section' in line:
                continue
            if line.startswith('----'):
                continue
            if '|' in line:
                cols = line.split('|')
                k, v = cols[0], cols[4]
            elif ' : ' in line:
                k, v = line.split(':')
            else:
                table.append(row)
                row = None
                state = State.searching
                continue
            k, v = k.strip(), v.strip()
            row[k] = v
            continue

    return table


def print_table(table: Table):
    print(f'{table.name}')
    for line in table:
        print(' ', line)
    print()


def main():
    parser = argparse.ArgumentParser(description='Sming CI log parser')
    parser.add_argument('filename', help='Log filename')
    parser.add_argument('-c', '--compare', help='Second log to compare')

    args = parser.parse_args()

    table1 = scan_log(args.filename)
    if args.compare is None:
        print_table(table1)
        return

    table2 = scan_log(args.compare)

    for row1 in table1.rows:
        target = row1[0]
        i = table2.find_row(target)
        if i < 0:
            print(f'** {target} NOT found in {table2.name}')
            continue
        row2 = table2.rows.pop(i)
        if row2 == row1:
            continue

        diff_table = Table(target)

        data = {'log': table1.name}
        for k, v in zip(table1.headings[1:], row1[1:]):
            data[k] = v
        diff_table.append(data)
        data = {'log': table2.name}
        for k, v in zip(table2.headings[1:], row2[1:]):
            data[k] = v
        diff_table.append(data)

        data = {'log': 'Difference'}
        for name, v1, v2 in zip(table1.headings[1:], row1[1:], row2[1:]):
            if v1 == v2:
                continue
            v1, v2 = int(v1, 0), int(v2, 0)
            data[name] = f'{v2-v1:+}'
        diff_table.append(data)

        print_table(diff_table)

    if table2.rows:
        print(f'** Targets not in {table1.name}')
        print_table(table2)


if __name__ == "__main__":
    main()
