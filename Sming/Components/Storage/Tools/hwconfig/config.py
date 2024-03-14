#
# Configuration object
#

import os, partition, storage, copy
from common import *
from builtins import classmethod

HW_EXT = '.hw'

def get_config_dirs():
    s = os.environ['HWCONFIG_DIRS']
    dirs = s.strip().replace('  ', ' ').split(' ')
    return dirs

def load_option_library():
    library = {}
    dirs = get_config_dirs()
    for d in dirs:
        filename = fixpath(d) + '/options.json'
        if os.path.exists(filename):
            data = json_load(filename)
            library.update(data)
    return library

def load_build_library():
    library = {}
    s = os.environ['HWCONFIG_BUILDSPECS']
    s = s.strip().replace('  ', ' ')
    if len(s) != 0:
        for f in s.split(' '):
            data = json_load(fixpath(f))
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
        # Device
        properties = self['Device']['properties']
        properties['type']['enum'] = list((storage.TYPES).keys())
        # Partition
        properties = self['Partition']['properties']
        properties['type']['enum'] = list((partition.TYPES).keys() - ['storage', 'internal'])
        properties['subtype']['enum'] = []
        # Add defined build targets and all available build fields
        self.builders = load_build_library()
        tgt = properties['build.target'] = self['Build']['properties']['target']
        tgt['enum'] = list(self.builders.keys())
        for builder in self.builders.values():
            for k, v in builder['properties'].items():
                properties['build.' + k] = v

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
        self.name = ''
        self.comment = ''

    def __str__(self):
        return "'%s' for %s" % (self.name, self.arch)

    @classmethod
    def from_name(cls, name):
        """Create configuration given its name and resolve options."""
        config = Config()
        config.load(name)
        options = os.environ.get('HWCONFIG_OPTS', '').replace(' ', '')
        if options != '':
            config.parse_options(options.split(','))
        config.resolve_expressions()
        config.partitions.sort()
        return config

    @classmethod
    def from_json(cls, json, options = None):
        config = Config()
        config.parse_dict(copy.deepcopy(json))
        if options is not None:
            config.parse_options(options)
        config.resolve_expressions()
        config.partitions.sort()
        return config

    def load(self, name):
        """Load a configuration recursively."""
        filename = find_config(name)
        self.depends.append(filename)
        data = json_load(filename)
        self.parse_dict(data)
        self.name = data.get('name', '')
        self.comment = data.get('comment', '')

    def parse_options(self, options):
        """Apply any specified options.
        
        Each option can be applied more than once to ensure overrides work as expected.
        """
        for option in options:
            self.options.append(option)
            data = self.option_library.get(option)
            if data is None:
                raise InputError("Option '%s' undefined" % option)
            # Don't modify library entries
            temp = copy.deepcopy(data)
            temp.pop('description', None)
            self.parse_dict(temp)

    def resolve_expressions(self):
        self.partition_table_offset = eval(str(self.partition_table_offset))
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
            elif k == 'bootloader_size':
                self.bootloader_size = parse_int(v)
            elif k == 'partition_table_offset':
                self.partition_table_offset = v
            elif k == 'devices':
                self.devices.parse_dict(v)
            elif k != 'name' and k != 'comment':
                raise InputError("Unknown config key '%s'" % k)

        if not partitions is None:
            self.partitions.parse_dict(partitions, self.devices)

    def dict(self):
        res = {}

        res['name'] = self.name
        if hasattr(self, 'comment'):
            res['comment'] = self.comment
        res['arch'] = self.arch
        res['options'] = self.options
        res['bootloader_size'] = size_format(self.bootloader_size)
        res['partition_table_offset'] = addr_format(self.partition_table_offset)
        res['devices'] = self.devices.dict()
        res['partitions'] = self.partitions.dict()
        return res

    def to_json(self):
        return to_json(self.dict())

    def buildVars(self):
        dict = {}

        dict['SMING_ARCH_HW'] = self.arch
        dict['PARTITION_TABLE_OFFSET'] = addr_format(self.partition_table_offset)
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
        if self.partition_table_offset % partition.FLASH_SECTOR_SIZE != 0:
            raise InputError("Partition table offset not aligned to flash sector")
        self.devices.verify()
        self.partitions.verify(self, secure)

    def map(self):
        return partition.Map(self)

    @classmethod
    def from_binary(cls, b):
        res = Config()
        res.name = 'from binary'
        res.arch = os.environ.get('SMING_ARCH', 'Unknown')
        res.partition_table_offset = 0
        res.partitions.parse_binary(b, res.devices)
        return res
