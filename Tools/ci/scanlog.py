#!/usr/bin/env python
#
# Python application to scan CI build logs and extract useful information
#
# Logs can be in two formats:
#     1. Logs fetched using `gh run view --log nnn` have all jobs in single file.
#        Each line is structured as "{job}\t{step}\t{datetime} {line}"
#     2. Logs captured manually via web browser are for a single job.
#        Each line is structured as "{datetime} {line}"
#
# The most useful is (1) since it can be scripted, and requires fewer steps.
# The two separate logs (regular and esp32 runs) can also be combined and processed as a single file.
#
# Use cases:
#
#   Generate table of memory usage per sample
#   Given a second log file, compare each run to show summary of differences in memory usage
#   Pull out all warnings
#       There are many duplicates in a run and across runs, so we can filter these
#       to produce a definitive list.
#

import argparse
import os
import sys
import re
import subprocess
import json
from enum import Enum

class Table:
    COL_SEP = '  '

    def __init__(self):
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


class Job:
    def __init__(self, log: "Log", name: str):
        self.log = log
        self.name = name
        self.table: Table = Table()
        self.warnings: dict[str, str] = {} # De-duplicated warnings
        self.warning_count: int = 0 # Includes duplicates

    @property
    def caption(self):
        return f'{self.log.name}: {self.name}'


class Log:
    def __init__(self, name: str):
        self.name = os.path.splitext(name)[0]
        self.jobs: list[Job] = []


class PathPrefix:
    # Paths vary by platform, so normalise them
    IGNORE_PREFIX = [
        # Linux
        '/home/runner/projects/',
        '/home/runner/work/Sming/Sming/Sming/',
        '/opt/',
        # MacOS
        '/Users/runner/projects/',
        '/Users/runner/work/Sming/Sming/Sming/',
        # Windows
        '/d/a/Sming/Sming/projects/',
        '/d/a/Sming/Sming/Sming/',
        'd:/a/Sming/Sming/projects/',
        'd:/a/Sming/Sming/Sming/',
    ]
    re_remove = re.compile('|'.join(f'^{s}' for s in IGNORE_PREFIX), re.IGNORECASE)
    re_subst = re.compile(r'^d:/opt/esp-idf-\d.\d', re.IGNORECASE)

    @staticmethod
    def normalise(line: str) -> str:
        s = PathPrefix.re_remove.sub('', line)
        s = PathPrefix.re_subst.sub('esp-idf', s)
        return s


def normalise_path(line: str) -> str:
    line = line.replace('\\', '/')
    line = PathPrefix.normalise(line)
    return os.path.normpath(line)


def scan_log(filename: str) -> Log:
    class State(Enum):
        searching = 1
        building = 2
        linking = 3
    BUILD_PREFIX = 'Building '
    log = Log(filename)
    job = None
    warnings = {}
    state = State.searching
    target = None
    row = None

    def finish_job():
        if job is None:
            return

        nonlocal row
        if row is not None:
            job.table.append(row)
            row = None

        nonlocal warnings
        job.warnings = warnings
        warnings = {}

    # Second figure in warning lines not reliable, remove it
    warning_split = re.compile(r':(\d+): ?(\d+: )?(warning: )')

    sys.stderr.write(f'Scanning {filename}\n')

    logfile = open(filename, 'rb')
    for line in logfile:
        line = line.decode('utf-8-sig').strip()
        # Replace typographical quotes with normal ones to ensure equivalence
        line = re.sub(r"‘|’", "'", line)
        job_name = None
        if not line[:4].isdigit(): # Not a date
            if not line.startswith('build'):
                continue
            job_name, _, line = line.partition('\t')
            job_name = re.search(r'\((.+?)\)', job_name)[1]
            step, _, line = line.partition('\t')
            if not step.startswith('Build and test'):
                continue
            if job and job.name != job_name:
                finish_job()
                sys.stderr.write(f'\r{job.name} ...\033[K')
                job = None
        if job is None:
            job = Job(log, job_name)
            log.jobs.append(job)
            state = State.searching
        dtstr, _, line = line.partition(' ')
        if not dtstr:
            continue
        if ': warning:' in line:
            job.warning_count += 1
            s = line.removeprefix('from ')
            x = warning_split.split(line)
            if len(x) == 5:
                s = normalise_path(f'{x[0]}:{x[1]}')
                lines = warnings.setdefault(s, set())
                lines.add(x[4])
            else:
                warnings.setdefault(s, set())
            continue
        if state == State.searching:
            if not line.startswith(BUILD_PREFIX):
                continue
            if 'clib-App' not in line:
                continue
            c = normalise_path(line[len(BUILD_PREFIX):])
            target, _, _ = c.partition('/out/')
            state = State.building
            continue
        if state == State.building:
            if line.startswith(f'{os.path.basename(target)}: Linking'):
                state = State.linking
                row = None
                continue
        if state == State.linking:
            if row is None:
                if line.startswith('----'):
                    row = {'target': target}
                continue
            if '|' in line:
                cols = line.split('|')
                k, v = cols[0], cols[4]
            elif ' : ' in line:
                k, v = line.split(':')
            else:
                job.table.append(row)
                row = None
                state = State.searching
                continue
            k, v = k.strip(), v.strip()
            row[k] = v

    finish_job()

    sys.stderr.write('\r\033[K')
    log.jobs.sort(key=lambda job: job.name)

    return log


def print_table(table: Table):
    for line in table:
        print(' ', line)
    print()


def print_warnings(log: Log, exclude_file: str):
    exclude = None
    if exclude_file is not None:
        with open(exclude_file, 'r') as f:
            s = '|'.join(line.strip() for line in f)
            exclude = re.compile(s, re.IGNORECASE)

    warnings = {}
    total_warning_count = 0
    for job in log.jobs:
        total_warning_count += job.warning_count
        for location, details in job.warnings.items():
            location_warnings = warnings.setdefault(location, {})
            for det in details:
                x = location_warnings.setdefault(det, [])
                x.append(job.name)

    print(f'{total_warning_count} warnings found in {len(warnings)} unique locations.')

    exclude_count = 0
    if exclude:
        unfiltered_warnings = warnings
        warnings = {}
        for location, details in unfiltered_warnings.items():
            filtered_details = []
            for det in details:
                if not exclude.match(f'{location}\t{det}'):
                    filtered_details.append(det)
            if filtered_details:
                warnings[location] = filtered_details
            else:
                exclude_count += 1
        print(f'{exclude_count} locations excluded.')

    loc_width = min(2 + max(len(loc) for loc in warnings), 80)
    loc_pad = ''.ljust(loc_width)
    for location in sorted(warnings, key=lambda s: s.lower()):
        if len(location) > loc_width:
            print(f'\t{location}')
            locstr = loc_pad
        else:
            locstr = f'{location}'.ljust(loc_width)
        for det in sorted(warnings[location]):
            print(f'\t{locstr}{det}')
            locstr = loc_pad


def fetch_logs(filename: str, repo: str = None, branch: str = None):
    if os.path.exists(filename):
        sys.stderr.write(f'{filename} exists, skipping download.\n')
        return
    def get_args(cmd: str):
        args = ['gh', 'run', cmd]
        if repo:
            args.append(f'-R={repo}')
        return args
    args = get_args('list')
    if branch:
        args.append(f'-b={branch}')
    args.append('--json=displayTitle,headBranch,number,name,databaseId,headSha,conclusion')
    r = subprocess.run(args, capture_output=True, encoding='utf-8')
    r.check_returncode()
    data = json.loads(r.stdout)

    joblist = []
    for job in data:
        if '(CI)' not in job['name']:
            continue
        if joblist and job['headSha'] != joblist[0]['headSha']:
            break
        joblist.append(job)

    with open(filename, 'w') as f:
        sys.stderr.write(f'Creating {filename}...\n')
        for job in joblist:
            job_id = job['databaseId']
            sys.stderr.write(f'Fetching {job_id}: "{job["displayTitle"]}" - {job["headBranch"]} - {job["name"]} - {job["conclusion"]}\n')
            try:
                args = get_args('view') + ['--log', str(job_id)]
                r = subprocess.run(args, stdout=f, encoding='utf-8')
                r.check_returncode()
            except:
                os.unlink(filename)
                raise


def print_diff(log1: Log, log2: Log):
    for job1 in log1.jobs:
        job_printed = False
        try:
            job2 = next(job for job in log2.jobs if job.name == job1.name)
        except StopIteration:
            print(f'** job "{job1.name}" not found in "{log2.name}"')
            continue
        table1 = job1.table
        table2 = job2.table
        for row1 in table1.rows:
            target = row1[0]
            i = table2.find_row(target)
            if i < 0:
                print(f'** {target} NOT found in {table2.caption}')
                continue
            row2 = table2.rows.pop(i)
            if row2 == row1:
                continue

            diff_table = Table()

            data = {'log': job1.log.name}
            for k, v in zip(table1.headings[1:], row1[1:]):
                data[k] = v
            diff_table.append(data)
            data = {'log': job2.log.name}
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

            if not job_printed:
                print(f'{job1.name}:')
                job_printed = True
            print_table(diff_table)

    if table2.rows:
        print(f'** Targets not in {job1.name}')
        print_table(table2)


def main():
    parser = argparse.ArgumentParser(description='Sming CI log parser')
    parser.add_argument('filename', help='Log filename to read/write')
    parser.add_argument('-f', '--fetch', action='store_true', help='Fetch most recent CI runs from repo')
    parser.add_argument('-R', '--repo', help='Override default repo for fetch')
    parser.add_argument('-b', '--branch', help='Specify branch to fetch')
    parser.add_argument('-c', '--compare', help='Second log to compare')
    parser.add_argument('-w', '--warnings', action='store_true', help='Summarise warnings')
    parser.add_argument('-x', '--exclude', help='File containing source locations to exclude')

    args = parser.parse_args()

    if args.fetch:
        fetch_logs(args.filename, repo=args.repo, branch=args.branch)

    log1 = scan_log(args.filename)
    if args.compare is None:
        if args.warnings:
            print_warnings(log1, args.exclude)
        else:
            for job in log1.jobs:
                print(job.caption)
                print_table(job.table)
        return

    log2 = scan_log(args.compare)
    print_diff(log1, log2)


if __name__ == "__main__":
    main()
