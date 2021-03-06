#
# Configuration object
#

import os, partition, storage
from rjsmin import jsmin
from common import *
from builtins import classmethod

def findConfig(name):
    dirs = os.environ['HWCONFIG_DIRS'].split(' ')
    for d in dirs:
        path = os.path.join(fixpath(d), name + '.hw')
        if os.path.exists(path):
            return path
    raise InputError("Config '%s' not found" % name)

class Config(object):
    def __init__(self):
        self.partitions = partition.Table()
        self.devices = storage.List()
        self.depends = []

    def __str__(self):
        return "'%s' for %s" % (self.name, self.arch)

    @classmethod
    def from_name(cls, name):
        config = Config()
        config.load(name)
        return config

    def load(self, name):
        filename = findConfig(name)
        self.depends.append(filename)
        din = open(filename).read()
        data = json.loads(jsmin(din))
        self.parse_dict(data)

    def parse_dict(self, data):
        base_config = data.get('base_config')
        if base_config is not None:
            self.load(base_config)
            del data['base_config']

        for k, v in data.items():
            if k == 'name':
                self.name = v
            elif k == 'arch':
                self.arch = v
            elif k == 'partition_table_offset':
                self.partitions.offset = parse_int(v)
            elif k == 'devices':
                self.devices.parse_dict(v)
            elif k == 'comment':
                self.comment = v
            elif k != 'partitions':
                raise InputError("Unknown config key '%s'" % k)

        v = data.get('partitions')
        if not v is None:
            self.partitions.parse_dict(v, self.devices)

    def dict(self):
        res = {}

        res['name'] = self.name
        if hasattr(self, 'comment'):
            res['comment'] = self.comment
        res['arch'] = self.arch;
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
        self.partitions.verify(self.arch, secure)

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
