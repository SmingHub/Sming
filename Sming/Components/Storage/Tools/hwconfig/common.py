#
# Common functions and definitions
#

import sys, json, platform

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
    try:
        for letter, multiplier in [("k", 1024), ("m", 1024 * 1024)]:
            if v.lower().endswith(letter):
                return parse_int(v[:-1], keywords) * multiplier
        return int(v, 0)
    except ValueError:
        if keywords is None:
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
    if a != 0:
        for (val, suffix) in [(0x40000000, "G"), (0x100000, "M"), (0x400, "K")]:
            if a % val == 0:
                return "%d%s" % (a // val, suffix)
    return "0x%08x" % a


def quote(v):
    return '"' + v + '"'


def contains_whitespace(s):
    return ''.join(s.split()) != s


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

    def __init__(self, partition, message):
        super(ValidationError, self).__init__(
            "Partition %s invalid: %s" % (partition.name, message))

