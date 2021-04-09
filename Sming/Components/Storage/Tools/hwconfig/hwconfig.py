#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import common, argparse, os, partition
from common import *
from config import Config
from config import schema as config_schema

def openOutput(path):
    if path == '-':
        try:
            stdout_binary = sys.stdout.buffer  # Python 3
        except AttributeError:
            stdout_binary = sys.stdout
        return stdout_binary
    status("Writing to '%s'" % path)
    output_dir = os.path.abspath(os.path.dirname(path))
    os.makedirs(output_dir, exist_ok=True)
    return open(path, 'wb')


def handle_validate(args, config, part):
    # Validate resulting hardware configuration against schema
    try:
        from jsonschema import Draft7Validator
        inst = json_loads(config.to_json())
        v = Draft7Validator(config_schema)
        errors = sorted(v.iter_errors(inst), key=lambda e: e.path)
        if errors != []:
            for e in errors:
                critical("%s @ %s" % (e.message, e.path))
            sys.exit(3)
    except ImportError as err:
        critical("\n** WARNING! %s: Cannot validate '%s', please run `make python-requirements **\n\n" % (str(err), args.input))


def handle_flashcheck(args, config, part):
    # Expect list of chunks, such as "0x100000=/out/Esp8266/debug/firmware/spiff_rom.bin 0x200000=custom.bin"
    list = args.expr.split()
    if len(list) == 0:
        raise InputError("No chunks to flash!")
    for e in list:
        addr, filename = e.split('=')
        addr = int(addr, 0)
        part = config.partitions.find_by_address(config.devices[0], addr)
        if part is None:
            raise InputError("No partition contains address 0x%08x" % addr)
        if part.address != addr:
            raise InputError("Address 0x%08x is within partition '%s', not at start (0x%08x)" % (addr, part.name, part.address))
        filesize = os.path.getsize(filename)
        if filesize > part.size:
            raise InputError("File '%s' is 0x%08x bytes, too big for partition '%s' (0x%08x bytes)" % (os.path.basename(filename), filesize, part.name, part.size))


def handle_partgen(args, config, part):
    # Generate partition table binary
    if not args.no_verify:
        status("Verifying partition table...")
        config.verify(args.secure)
    return config.partitions.to_binary(config.devices)


def handle_expr(args, config, part):
    # Evaluate expression against configuration data
    return str(eval(args.expr)).encode()


def main():
    parser = argparse.ArgumentParser(description='Sming hardware configuration utility')

    parser.add_argument('--no-verify', help="Don't verify partition table fields", action='store_true')
    parser.add_argument('--quiet', '-q', help="Don't print non-critical status messages to stderr", action='store_true')
    parser.add_argument('--secure', help="Require app partitions to be suitable for secure boot", action='store_true')
    parser.add_argument('--part', help="Name of partition to operate on")
    parser.add_argument('command', help='Action to perform', choices=['partgen', 'expr', 'validate', 'flashcheck'])
    parser.add_argument('input', help='Name of hardware configuration or path to binary partition table')
    parser.add_argument('output', help='Path to output file. Will use stdout if omitted.', nargs='?', default='-')
    parser.add_argument('expr', help='Expression to evaluate', nargs='?', default=None)

    args = parser.parse_args()

    common.quiet = args.quiet

    output = None
    input_is_binary = False
    if os.path.exists(args.input):
        inputData = open(args.input, "rb").read()
        input_is_binary = inputData[0:2] == partition.Entry.MAGIC_BYTES
        if input_is_binary:
            config = Config.from_binary(inputData)
        else:
            raise InputError("File '%s' not recognised as partition table" % args.input)
    else:
        config = Config.from_name(args.input)
        partitions = config.partitions

    # Locate any supplied partition by name
    part = None
    if args.part is not None:
        part = partitions.find_by_name(args.part)
        if part is None:
            return

    output = globals()['handle_' + args.command](args, config, part)

    if output is not None:
        openOutput(args.output).write(output)


if __name__ == '__main__':
    try:
        main()
    except InputError as e:
        print("** ERROR! %s" % e, file=sys.stderr)
        sys.exit(2)
