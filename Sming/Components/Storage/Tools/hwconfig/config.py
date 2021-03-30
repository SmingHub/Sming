#
# Configuration object
#

import os, partition, storage, copy
from common import *
from builtins import classmethod

HW_EXT = '.hw'

def get_config_dirs():
    s = os.environ['HWCONFIG_DIRS']
    dirs = s.replace('  ', ' ').split(' ')
    return dirs

def load_option_library():
    library = {}
    dirs = get_config_dirs()
    for d in dirs:
        filename = fixpath(d) + '/options.json'
        if os.path.exists(filename):
            with open(filename) as f:
                data = json_loads(f.read())
                library.update(data)
    return library

def get_config_list():
    list = {}
    dirs = get_config_dirs()
    for d in reversed(dirs):
        for f in os.listdir(fixpath(d)):
            if f.endswith(HW_EXT):
                n = os.path.splitext(f)[0]
                list[n] = d + '/' + f
    return list

def find_config(name):
    dirs = get_config_dirs()
    for d in dirs:
        path = fixpath(d) + '/' + name + HW_EXT
        if os.path.exists(path):
            return path
    raise InputError("Config '%s' not found" % name)


class Schema(dict):
    def __init__(self, filename):
        self.schema = json_load(filename)
        # Config
        properties = self['Config']['properties']
        properties['base_config']['enum'] = list(get_config_list().keys())
        optionlib = load_option_library()
        options = properties['options']['enum'] = {}
        for k, v in optionlib.items():
            options[k] = v['description']
        # Device
        properties = self['Device']['properties']
        properties['type']['enum'] = list((storage.TYPES).keys())
        # Partition
        properties = self['Partition']['properties']
        properties['type']['enum'] = list((partition.TYPES).keys() - ['storage', 'internal'])
        properties['subtype']['enum'] = []


    def __getitem__(self, name):
        return self.schema['definitions'][name]


schema = Schema(os.environ['HWCONFIG_SCHEMA'])

class Config(object):
    def __init__(self):
        self.devices = storage.List()
        self.partitions = partition.Table(self.devices)
        self.depends = []
        self.options = []
        self.option_library = load_option_library()
        self.base_config = None

    def __str__(self):
        return "'%s' for %s" % (self.name, self.arch)

    @classmethod
    def from_name(cls, name):
        """Create configuration given its name and resolve options
        """
        config = Config()
        config.load(name)
        options = os.environ.get('HWCONFIG_OPTS', '').replace(' ', '')
        if options != '':
            config.parse_options(options.split(','))
        config.resolve_expressions()
        config.partitions.sort()
        return config

    @classmethod
    def from_json(cls, json, options = []):
        config = Config()
        config.parse_dict(copy.deepcopy(json))
        config.parse_options(options)
        config.resolve_expressions()
        config.partitions.sort()
        return config

    def load(self, name):
        """Load a configuration recursively
        """
        filename = find_config(name)
        self.depends.append(filename)
        data = json_load(filename)
        self.parse_dict(data)

    def parse_options(self, options):
        """Apply any specified options, each option is applied only once
        """
        for option in options:
            if option in self.options:
                continue
            self.options.append(option)
            data = self.option_library.get(option)
            if data is None:
                raise InputError("Option '%s' undefined" % option)
            # Don't modify library entries
            temp = copy.deepcopy(data)
            temp.pop('description', None)
            self.parse_dict(temp)

    def resolve_expressions(self):
        self.partitions.offset = eval(str(self.partitions.offset))
        for p in self.partitions:
            p.resolve_expressions()

    def parse_dict(self, data):
        base_config = data.pop('base_config', None)
        if self.base_config is None:
            self.base_config = base_config
        if base_config is not None:
            self.load(base_config)

        self.parse_options(data.pop('options', []))

        # We'll process partitions after other settings
        partitions = data.pop('partitions', None)

        for k, v in data.items():
            if k == 'arch':
                self.arch = v
            elif k == 'partition_table_offset':
                self.partitions.offset = v
            elif k == 'devices':
                self.devices.parse_dict(v)
            elif k != 'name' and k != 'comment':
                raise InputError("Unknown config key '%s'" % k)

        self.name = data.get('name', '')
        self.comment = data.get('comment', '')

        if not partitions is None:
            self.partitions.parse_dict(partitions, self.devices)

    def dict(self):
        res = {}

        res['name'] = self.name
        if hasattr(self, 'comment'):
            res['comment'] = self.comment
        res['arch'] = self.arch;
        res['options'] = self.options
        res['partition_table_offset'] = self.partitions.offset_str()
        res['devices'] = self.devices.dict()
        res['partitions'] = self.partitions.dict()
        return res

    def to_json(self):
        return to_json(self.dict())

    def buildVars(self):
        dict = {}

        dict['SMING_ARCH_HW'] = self.arch
        dict['PARTITION_TABLE_OFFSET'] = self.partitions.offset_str()
        dict['PARTITION_TABLE_LENGTH'] = "0x%04x" % partition.MAX_PARTITION_LENGTH
        dict['SPIFLASH_PARTITION_NAMES'] = " ".join(p.name for p in filter(lambda p: p.device == self.devices[0], self.partitions))
        dict['HWCONFIG_DEPENDS'] = " ".join(self.depends)
        dict.update(self.devices.buildVars())
        dict.update(self.partitions.buildVars())

        res = "# Generated from hardware configuration '%s'\r\n" % self.name
        for k, v in dict.items():
            res += "%s = %s\r\n" % (k, v)
        return res

    def verify(self, secure):
        self.devices.verify()
        self.partitions.verify(self.arch, self.devices[0], secure)

    def map(self):
        return partition.Map(self.partitions, self.devices)

    @classmethod
    def from_binary(cls, b):
        res = Config()
        res.name = 'from binary'
        res.arch = os.environ.get('SMING_ARCH', 'Unknown')
        res.partitions.offset = 0
        res.partitions.parse_binary(b, res.devices)
        return res
