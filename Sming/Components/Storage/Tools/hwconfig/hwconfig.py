#!/usr/bin/env python3
#
# Sming hardware configuration tool
#

import common, argparse, os, partition
from common import *
from config import Config

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


def main():
    parser = argparse.ArgumentParser(description='Sming hardware configuration utility')

    parser.add_argument('--no-verify', help="Don't verify partition table fields", action='store_true')
    parser.add_argument('--quiet', '-q', help="Don't print non-critical status messages to stderr", action='store_true')
    parser.add_argument('--secure', help="Require app partitions to be suitable for secure boot", action='store_true')
    parser.add_argument('--part', help="Name of partition to operate on")
    parser.add_argument('command', help='Action to perform', choices=['partgen', 'expr', 'validate'])
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
    if not input_is_binary:
        config = Config.from_name(args.input)
        partitions = config.partitions

    # Define local so it's available for eval()
    if args.part is not None:
        part = partitions.find_by_name(args.part)
        if part is None:
            return

    if args.command == 'validate':
        try:
            from jsonschema import Draft7Validator
            from jsonschema.exceptions import ValidationError
        except ImportError:
            critical("hwconfig: `jsonschema` is not installed. Please run `make python-requirements`")
            sys.exit(1)
        inst = json.loads(config.to_json())
        schema = json.load(open(args.expr))
        v = Draft7Validator(schema)
        errors = sorted(v.iter_errors(inst), key=lambda e: e.path)
        if errors != []:
            for e in errors:
                critical("%s @ %s" % (e.message, e.path))
            sys.exit(3)
    elif args.command == 'partgen':
        if not args.no_verify:
            status("Verifying partition table...")
            config.verify(args.secure)
        output = config.partitions.to_binary(config.devices)
    elif args.command == 'expr':
        output = str(eval(args.expr)).encode()
    else:
        raise InputError('Unknown command: %s' % args.command)

    if output is not None:
        openOutput(args.output).write(output)


if __name__ == '__main__':
    try:
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
