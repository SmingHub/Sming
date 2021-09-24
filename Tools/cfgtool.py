#!/usr/bin/env python3
#
# Sming configuration management tool
#
# Used by build system to enable compatibility with Kconfig
#
#

import argparse, configparser, os, sys, kconfiglib


def load_config_vars(filename):
    parser = configparser.ConfigParser()
    parser.optionxform = str # preserve case
    with open(filename) as f:
        data = "[config]\n" + f.read()
    data = data.replace(' := ', ' = ')
    parser.read_string(data)
    return parser['config']


def main():
    parser = argparse.ArgumentParser(description='Sming configuration management tool')
    parser.add_argument('--to-kconfig', help="Convert Sming configuration to Kconfig format", action='store_true')
    parser.add_argument('--from-kconfig', help="Convert Kconfig configration to Sming format", action='store_true')
    parser.add_argument('config_file', help='Source configuration file')

    args = parser.parse_args()

    conf = kconfiglib.Kconfig(os.environ['KCONFIG'])
    if args.to_kconfig:
        src = load_config_vars(args.config_file)
        for k, v in src.items():
            c = conf.syms.get(k)
            if not c:
                continue
            if c.type is kconfiglib.BOOL:
                v = 'y' if v == '1' else 'n'
            c.set_value(v)
        conf.write_config(os.environ['KCONFIG_CONFIG'])
    elif args.from_kconfig:
        conf.load_config(os.environ['KCONFIG_CONFIG'])
        dst = load_config_vars(args.config_file)
        varnames = set(dst.pop('CACHED_VAR_NAMES').split())

        for k, sym in conf.syms.items():
            if sym.type is kconfiglib.UNKNOWN:
                continue
            if sym.env_var is not None:
                continue
            v = str(sym.user_value) if sym.user_value else ""
            if sym.type is kconfiglib.BOOL:
                v = "1" if v in ['y', '2'] else "0"
            dst[sym.name] = v
            varnames.add(sym.name)
        with open(args.config_file, "w") as f:
            for k, v in dst.items():
                f.write("%s=%s\n" % (k, v))
            f.write("\n")
            varnames = list(varnames)
            varnames.sort()
            f.write("CACHED_VARE_NAMES := " + " ".join(varnames))
    else:
        raise RuntimeError("No command specified")


if __name__ == '__main__':
    try:
        main()
    except Exception as e:
        print("** ERROR! %s" % e, file=sys.stderr)
        sys.exit(2)
