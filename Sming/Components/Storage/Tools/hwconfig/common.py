#
# Common functions and definitions
#

import sys, json, platform
from rjsmin import jsmin
from collections import OrderedDict

quiet = False

def status(msg):
    """ Print status message to stderr """
    if not quiet:
        critical(msg)


def critical(msg):
    """ Print critical message to stderr """
    sys.stderr.write(msg)
    sys.stderr.write('\n')


def fixpath(path):
    """ Paths in Windows can get a little weird """
    if len(path) > 2 and path[1] != ':' and platform.system() == 'Windows' and path[2] == '/':
        return path[1] + ':' + path[2:]
    return path


def parse_int(v, keywords=None):
    """Generic parser for integer fields - int(x,0) with provision for
    k/m/K/M suffixes and 'keyword' value lookup.
    """
    if not isinstance(v, str):
        return v
    if keywords is None or len(keywords) == 0:
        try:
            for letter, multiplier in [("k", 1024), ("m", 1024 * 1024), ("g", 1024 * 1024 * 1024)]:
                if v.lower().endswith(letter):
                    return round(float(v[:-1]) * multiplier)
            return int(v, 0)
        except ValueError:
            raise InputError("Invalid field value %s" % v)
    try:
        return keywords[v.lower()]
    except KeyError:
        raise InputError("Value '%s' is not valid. Known keywords: %s" % (v, ", ".join(keywords)))


def stringnum(s):
    """Return number if s contains only digits, otherwise return the string
    """
    return int(s) if s.isdigit() else s


def addr_format(a):
    return "0x%08x" % a


def size_format(a):
    if a == 0:
        return '0'
    for (val, suffix) in [(0x40000000, "G"), (0x100000, "M"), (0x400, "K")]:
        if a % val == 0:
            return "%d%s" % (a // val, suffix)
    return "0x%08x" % a


def size_frac_str(a):
    KB = 1024
    MB = KB * 1024
    GB = MB * 1024
    if a >= GB:
        div = GB
        unit = 'G'
    elif a >= MB:
        div = MB
        unit = 'M'
    else:
        div = KB
        unit = 'K'
    if a % div == 0:
        return "%u%s" % (a // div, unit)
    else:
        return "%.2f%s" % (a / div, unit)


def quote(v):
    return '"' + v + '"'


def contains_whitespace(s):
    return ''.join(s.split()) != s


def json_loads(s):
    return json.loads(jsmin(s), object_pairs_hook=OrderedDict)

def json_load(filename):
    with open(filename) as f:
        return json_loads(f.read())

def json_save(data, filename):
    with open(filename, "w") as f:
        json.dump(data, f, indent=4)

def to_json(obj):
    return json.dumps(obj, indent=4)


def lookup_keyword(t, keywords):
    for k, v in keywords.items():
        if t == v:
            return k
    return "%d" % t


class InputError(RuntimeError):
    def __init__(self, e):
        super(InputError, self).__init__(e)


class ValidationError(InputError):
    def __init__(self, obj, message):
        super(ValidationError, self).__init__("%s.%s '%s' invalid: %s" % (type(obj).__module__, type(obj).__name__, obj.name, message))
        self.obj = obj
