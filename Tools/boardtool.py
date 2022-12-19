#!/usr/bin/env python3
#
# Sming board configuration management tool
#
# Provides support for board features and pin configuration.
#
#

import argparse, os, sys, json, re
import natsort as ns

sys.path.insert(1, os.path.expandvars('${SMING_HOME}/../Tools/Python'))
from rjsmin import jsmin

verbose = False

def find(lst, item):
    res = [x for x in lst if item == x]
    return res[0] if res else None


class Group(object):
    """A group of items from pindefs, such as io_mux, rtc_mux, etc."""

    def __init__(self, name):
        self.fieldnames = None
        self.name = name
        self.entries = []

    def __repr__(self):
        return self.name


class Entry(object):
    """A group entry containing row of information fields from pindefs"""

    def __init__(self, group, fields):
        self.group = group
        self.fields = tuple(fields)

    def __getattr__(self, name):
        if name == 'fieldnames':
            return self.group.fieldnames
        return self.__getitem__(name)

    def __getitem__(self, item):
        """Allow entry access by name or index."""
        if isinstance(item, str):
            try:
                item = self.fieldnames.index(item)
            except ValueError:
                return None
        return self.fields[item]

    def __repr__(self):
        return str(self.fields)


class Signal(object):
    """Describes a peripheral signal which can be connected to a pin."""

    def __init__(self, entry, name, type):
        self.entries = [entry]
        self.name = name
        self.type = type
        self.peripheral = None

    def __repr__(self):
        return f"{self.name}"

    def __eq__(self, other):
        if isinstance(other, str):
            return other == self.name
        else:
            return self.name == other.name


class Pin(object):
    """Describes a physical pin using GPIO numbering."""

    def __init__(self, gpio, name, type):
        self.gpio = gpio
        self.name = name or f"GPIO{gpio}"
        self.signals = []
        self.type = type

    def __repr__(self):
        # return f"{self.gpio}: " + ", ".join(f"{s}" for s in self.signals)
        return f"{self.gpio}: " + ", ".join(f"{s} ({s.group})" for s in self.signals)


class Peripheral(object):
    """Describes a peripheral with associated signals."""

    def __init__(self, name, title, sigmask, default):
        self.name = name
        self.title = title
        self.sigmask = re.compile(sigmask) if sigmask else None
        self.default = default
        self.signals = []
        self.swap = None

    def help(self):
        """Obtain help text displayed in menus"""
        res = []
        if self.title:
            res += [f"{self.title}."]
        res += [sig.name for sig in self.signals]
        return res


class Config(object):
    """Contains parsed SOC pin, signal and peripheral information."""

    def __init__(self, arch, variant, name):
        self.arch = arch
        self.variant = variant
        self.name = name
        self.default = None
        self.signals = []
        self.peripherals = []

    def __repr__(self):
        return self.name

    @classmethod
    def load_file(cls, filename):
        if verbose:
            print(f"Loading '{os.path.basename(filename)}'", file=sys.stderr)
        s = open(filename, 'r').read()
        dn = os.path.dirname(filename)
        arch = os.path.basename(dn)
        spec = json.loads(jsmin(s))
        config = Config(arch, spec['variant'], spec['name'])

        pindefs = filename.replace('-soc.json', '-pindefs.txt')
        with open(pindefs) as f:
            lines = f.readlines()
            config.parse_pindefs(lines)

        natsort_key = ns.natsort_keygen(key = lambda sig: sig.name, alg=ns.LOWERCASEFIRST)
        config.signals.sort(key = natsort_key)

        config.parse_peripherals(spec['peripherals'])
        return config

    def parse_pindefs(self, lines):
        groups = {}
        fieldnames = None

        for line in lines:
            line = line.strip()
            if line == '' or line.startswith(('#', ';', '//')):
                continue
            if line[0] == '[':
                groupname = line[1:len(line)-1]
                group = groups[groupname] = Group(groupname)
                fieldnames = None
                continue
            if fieldnames is None:
                fieldnames = group.fieldnames = line.split()
                continue
            fields = []
            for i in range(len(fieldnames)-1):
                field, sep, line = line.partition(' ')
                fields.append(field)
            fields.append(line)
            entry = Entry(group, fields)
            group.entries.append(entry)

        self.pins = {}
        for name, group in groups.items():
            if name == 'gpio_matrix':
                self.parse_gpio_matrix(group)
            else:
                self.parse_iomux(group)

    def parse_iomux(self, group):
        for e in group.entries:
            gpio = int(e.gpio)
            pin = self.pins.get(gpio)
            if pin is None:
                pin = self.pins[gpio] = Pin(gpio, e.pad, 'I' if 'I' in e.notes else 'IO')
            p = re.compile("a|f[0-9]+")
            for n in filter(p.match, group.fieldnames):
                sig_name = e[n]
                if sig_name is None or sig_name == '-':
                    continue
                sig = find(self.signals, sig_name)
                if sig is None:
                    sig = Signal(e, sig_name, pin.type)
                    self.signals.append(sig)
                elif e not in sig.entries:
                    sig.entries.append(e)
                if sig not in pin.signals:
                    pin.signals.append(sig)
            p = re.compile("ff[0-9]+")
            for n in filter(p.match, group.fieldnames):
                sig = Signal(e, f"{e[n]}_{gpio}", pin.type)
                pin.signals.append(sig)
                self.signals.append(sig)

    def parse_gpio_matrix(self, group):
        """Signals can be switched to any GPIO."""
        def addSignal(e, f):
            sig = getattr(e, f)
            if sig == '-':
                return
            is_input = (f == 'input')
            sig = Signal(e, sig, 'I' if is_input else 'O')
            self.signals.append(sig)

            for i, pin in self.pins.items():
                if is_input or 'O' in pin.type:
                    if not sig in pin.signals:
                        pin.signals.append(sig)

        for e in group.entries:
            addSignal(e, 'input')
            addSignal(e, 'output')

    def parse_peripherals(self, spec):
        for name_spec, periphdef in spec.items():
            p = re.compile('\[([0-9]+)-([0-9]+)\]')
            matches = p.findall(name_spec)
            if len(matches) == 0:
                indexRange = range(-1, 0)
            else:
                m = matches[0]
                indexRange = range(int(m[0]), int(m[1]) + 1)

            title = periphdef.get('title')
            for idx in indexRange:
                name = p.sub(str(idx), name_spec)
                mask = periphdef.get('sigmask')
                if not mask:
                    break
                for tok in ['idx', 'name']:
                    mask = mask.replace(f'{{{tok}}}', str(locals()[tok]))
                per = Peripheral(name, title, mask, periphdef.get('default', {}))
                self.peripherals.append(per)
                for sig in self.signals:
                    if per.sigmask.match(sig.name):
                        per.signals.append(sig)
                        if sig.peripheral is not None:
                            raise RuntimeError(f"Attempted to assign signal {sig} to {per.name} but already assigned to {sig.peripheral.name}")
                        sig.peripheral = per
                per.swap = periphdef.get('swap')


        # Create 'Other' peripheral to catch undefined signals
        per = Peripheral('Other', 'Signals not associated with any peripheral', '', '')
        for sig in self.signals:
            if sig.peripheral is None:
                sig.peripheral = per
                per.signals.append(sig)
        if len(per.signals) != 0:
            self.peripherals.append(per)


def load_configs(socs = ''):
    """Load configurations for specific SOCs."""
    def match(path):
        if socs == '':
            return True
        base = os.path.basename(path)
        for s in socs.split():
            if base.startswith(s + '-'):
                return True
        return False
    res = []
    for f in filter(match, os.environ['SOC_CONFIG_FILES'].split()):
        res.append(Config.load_file(f))
    return res


def list_soc(args):
    configs = load_configs()
    configs.sort(key = lambda x: x.name)
    for config in configs:
        print()
        print(config.variant)
        print(f'         name: {config.name}')
        print(f'         arch: {config.arch}')
        print(f'         pins: {len(config.pins)}')
        print(f'      signals: {len(config.signals)}')
        print(f'  peripherals: {len(config.peripherals)}')
        if args.verbose:
            for per in config.peripherals:
                print(f'{per.name:>13}: {len(per.signals)} signals')


def load_active_config():
    return Config.load_file(os.environ['SOC_CONFIG_FILE'])


def generate_pinmenu(args):
    config = load_active_config()

    menu = [f'mainmenu "{config.name} Pin Configuration"']

    menu += ['menu "Peripherals"']
    for per in config.peripherals:
        menu += [
            f'  config PERIPH_{per.name}_ENABLE',
            f'    bool "{per.name}"',
            f'    default y',
            f'    help'
        ]
        menu += [f'      {s}' for  s in per.help()]
        if per.swap:
            menu += [
                f'  config PERIPH_{per.name}_SWAP_ENABLE',
                f'    bool "Swap {per.name} signals"',
                f'    default n',
                f'    depends on PERIPH_{per.name}_ENABLE',
                f'    help',
                f'      {per.swap["help"]}'
            ]
    menu += ['endmenu']

    menu += ['menu "Pin selections"']
    for n, pin in config.pins.items():
        menu += [
            f'  choice',
            f'    prompt "GP{pin.type} {n}"',
            f'    default PINSEL{int(n):02d}_{pin.signals[0]}'
        ]

        for sig in pin.signals:
            per = sig.peripheral
            alt = None
            if per.swap is not None:
                for a, b in per.swap['pins'].items():
                    if a == sig.name:
                        alt = find(config.signals, b)
                        break
                    if b == sig.name:
                        alt = find(config.signals, a)
                        break

            def addsig(sig, swap = None):
                nonlocal menu
                depends = f"PERIPH_{per.name}_ENABLE"
                if swap is not None:
                    depends += " &&"
                    if not swap:
                        depends += "!"
                    depends += f"PERIPH_{per.name}_SWAP_ENABLE"
                menu += [
                    f'    config PINSEL{int(n):02d}_{sig.name}',
                    f'      bool "{sig.name}"',
                    f'      depends on {depends}',
                    f'      help',
                    f'        Peripheral "{per.name}", {", ".join(entry.group.name for entry in sig.entries)}'
                ]

            if alt is None:
                addsig(sig)
            else:
                addsig(sig, False)
                addsig(alt, True)

        menu += ['  endchoice']
    menu += ['endmenu']

    for line in menu:
        print(line)


def list_default_pins(args):
    config = load_active_config()

    for per in config.peripherals:
        if not per.default:
            continue
        print(f'/* {per.name} */\n')
        for k, v in per.default.items():
            if v is None:
                gpio = 0xff
            else:
                sig, gpio = v
                if sig not in per.signals:
                    raise RuntimeError(f"Invalid signal '{sig}' for {per.name}")
                pin = config.pins[gpio]
                if sig not in pin.signals:
                    raise RuntimeError(f"Invalid signal '{sig}' for {pin.name}")
            print(f"#define SMG_PINDEF_{per.name}_{k} {gpio}")
        print('\n')


def main():
    global verbose

    parser = argparse.ArgumentParser(description='Sming board configuration tool')
    parser.add_argument('-v', '--verbose', help='Verbose output', action='store_true', default=False)
    subparsers = parser.add_subparsers()

    sub = subparsers.add_parser('list-soc', help="List available SOCs")
    sub.set_defaults(func=list_soc)

    sub = subparsers.add_parser('generate-pinmenu', help="Generate pin menu")
    sub.set_defaults(func=generate_pinmenu)

    sub = subparsers.add_parser('list-default-pins', help="List default pins")
    sub.set_defaults(func=list_default_pins)

    args = parser.parse_args()
    verbose = args.verbose
    fn = args.func
    if fn is not None:
        fn(args)


if __name__ == '__main__':
    main()
