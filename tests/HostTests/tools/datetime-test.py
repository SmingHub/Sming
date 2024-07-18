#!/usr/bin/python3
#
# Generate test data for DateTime module:
#
#   python datetime-test.py include/DateTimeData.h
#
#

import os
import sys
from dataclasses import dataclass
import datetime
import email.utils

"""
ISO datetimes can be strings in short form (i.e. no separators) or extended form (with separators)
For testing unix timestamp values give that and we'll translate it accordingly.

Python doesn't accept all formats specified by standard
In these cases, specify a tuple (str, python_str)

There are some gotchas with Python version:

- short-form unsupported before 3.11
- Windows craps itself when given negative timestamps

"""

def check_compatibility():
    version = 100 * sys.version_info.major + sys.version_info.minor
    if version < 311:
        raise DeprecationWarning("Require at least python 3.11")
    if sys.platform == 'win32':
        raise DeprecationWarning("Windows doesn't support negative unix timestamps!")


@dataclass
class Record:
    string: str
    string2: str
    timestamp: int
    milliseconds: int


def parse_http_date(value: str) -> Record:
    dt = email.utils.parsedate_to_datetime(value)
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=datetime.timezone.utc)
    return Record(value, email.utils.format_datetime(dt, usegmt=True), dt.timestamp(), 0)


def parse_iso_datetime(value: str | tuple | int) -> Record:
    if isinstance(value, int):
        dt = datetime.datetime.fromtimestamp(value, tz=datetime.timezone.utc)
        s = dt.isoformat().removesuffix('+00:00')
    elif isinstance(value, tuple):
        s = value[0]
        dt = datetime.datetime.fromisoformat(value[1])
    else:
        s = value
        dt = datetime.datetime.fromisoformat(value)
        dt = dt.astimezone(datetime.timezone.utc)
    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=datetime.timezone.utc)
    iso_string = dt.isoformat().replace('+00:00', 'Z')
    return Record(s, iso_string, dt.timestamp(), dt.microsecond // 1000)


def parse_iso_time(value: str) -> Record:
    t = datetime.time.fromisoformat(value)
    o = t.second + (t.minute * 60) + (t.hour * 60 * 60)
    if t.microsecond:
        iso_string = t.isoformat(timespec='milliseconds')
    else:
        iso_string = t.isoformat()
    return Record(value, iso_string, o, t.microsecond // 1000)


"""
Consider these as appropriate for HTTP and ISO date strings.

valid dates must include:
    all days
    min/max day numbers
    all month names
    min/max month numbers
    selection of years from min to max

invalid dates must include:
    bad day numbers (month-specific)
    leap years
    bad month numbers
    bad month names
    bad day names

times
    24-hour only
    hours:mins
    hours:mins:secs
    hours:mins:secs.milliseconds

After basic validation can use DateTime to generate strings and then verify they can be decoded back again.
"""

MAP_LIST = {
    'VALID_HTTP_DATE': (
        parse_http_date,
        [
            "Sun, 06 Nov 1994 08:49:37 GMT",
            "Sunday, 06 Nov 1994 08:49:37 GMT",
            "Sunday, 06-Nov-94 08:49:37 GMT",
            "Mon, 07 November 1994 00:00:00",
            "  1   JAN\t \r\n 	2000    23:59:59",
        ]),
    'VALID_ISO_DATETIME': (
        parse_iso_datetime,
        [
            "2024-01-31",
            "20240131",
            ("2024-01", "2024-01-01"),
            -0x7fffffff,
            -0x80000000,
            0,
            0x7fffffff,
            "1950-01-01",
            0x6613e40d,
            0x6615358c,
        ]),
    'VALID_ISO_DATETIME64': (
        parse_iso_datetime,
        [
            0x80000000,
            "2099-12-31T23:59:59",
            "2105-12-31T23:59:59",
            "2106-02-07T06:28:15",
            "2106-02-07T06:28:15",
            0x100000000,
            -0x80000000,
            "2024-05-22T06:28:15+07:30",
            "2024-05-22T06:28:15-23:45",
        ]),
    'VALID_ISO_TIME': (
        parse_iso_time,
        [
            "T18:47:12.123",
            "T18:47:12.123456",
            "T12:34:12",
            "T23:59:59",
            "12:34",
            "T2359",
        ]),
}


def main():
    check_compatibility()

    output = f"""
/*
    * DateTime test data
    *
    * File generated {datetime.datetime.now().ctime()}
    *
    */

// clang-format off

namespace
{{
struct TestDate {{
    const FlashString* stringToParse;
    const FlashString* expectedString;
    int64_t unixTimestamp;
    uint16_t milliseconds;
}};

"""

    index = 1
    for name, (parse, data) in MAP_LIST.items():
        records = [f'#define {name}_MAP(XX)']
        for value in data:
            r = parse(value)
            s = r.string.encode('unicode_escape').decode()
            records += [f'XX(DT_{index}, "{s}", "{r.string2}", {hex(int(r.timestamp))}LL, {r.milliseconds})']
            index += 1
        output += " \\\n  ".join(records)
        output += "\n\n"

    output += f"""
#define XX(tag, str, str2, ...) \\
    DEFINE_FSTR_LOCAL(STR1_##tag, str) \\
    DEFINE_FSTR_LOCAL(STR2_##tag, str2)
{"".join(f'  {name}_MAP(XX)\n' for name in MAP_LIST)}
#undef XX

#define XX(tag, str, str2, ...) {{&STR1_##tag, &STR2_##tag, ##__VA_ARGS__}},
{"".join(f'DEFINE_FSTR_ARRAY({name}, TestDate, {name}_MAP(XX))\n' for name in MAP_LIST)}
#undef XX

}} // namespace
"""

    if len(sys.argv) > 1:
        filename = sys.argv[1]
        print('Creating file', repr(filename))
        with open(filename, 'w') as f:
            print(output, file=f)
    else:
        print(output)


if __name__ == '__main__':
    main()
