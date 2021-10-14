#
# Storage devices
#

from common import *
import partition

TYPES = {
    "unknown": 0x00,
    "flash": 0x01,
    "spiram": 0x02,
    "sdcard": 0x03,
    "disk": 0x04,
    "file": 0x05,
}


def parse_type(value):
    if value == "":
        raise InputError("Field 'type' can't be left empty.")
    return parse_int(value, TYPES)


class List(list):
    def parse_dict(self, data):
        for name, v in data.items():
            if contains_whitespace(name):
                raise InputError("Device names may not contain spaces '%s'" % name)
            dev = self.find_by_name(name)
            if dev is None:
                dev = Device(name)
                self.append(dev)
            dev.parse_dict(v)

    def dict(self):
        res = {}
        for dev in self:
            res[dev.name] = dev.dict()
        return res

    def to_json(self):
        return to_json(self.dict())

    def buildVars(self):
        dict = {}
        dict['STORAGE_DEVICE_NAMES'] = " ".join(p.name for p in self)
        for p in self:
            dict.update(p.buildVars())
        return dict

    def __getitem__(self, item):
        """Access entries by name or index."""
        if isinstance(item, str):
            d = self.find_by_name(item)
            if d is None:
                raise ValueError("No device named '%s'" % item)
            return d
        return super().__getitem__(item)

    def find_by_name(self, name):
        for d in self:
            if d.name == name:
                return d
        return None

    def verify(self):
        for dev in self:
            dev.verify()

class Device(object):

    """Initialise a storage device."""
    def __init__(self, name, stype = 0, size = 0):
        self.name = name
        self.type = parse_type(stype)
        self.size = parse_int(size)

    def parse_dict(self, data):
        for k, v in data.items():
            if k == 'type':
                self.type = parse_int(v, TYPES)
            elif k == 'size':
                self.size = parse_int(v)
            elif k == 'mode':
                self.mode = v
            elif k == 'speed':
                self.speed = v
            else:
                raise InputError("Unknown storage field '%s'" % k)

    def dict(self):
        res = {}

        # Some fields are optional
        def tryAdd(k):
            if hasattr(self, k):
                res[k] = getattr(self, k)

        res['type'] = self.type_str()
        res['size'] = self.size_str()
        tryAdd('mode')
        tryAdd('speed')
        return res

    def to_json(self):
        return to_json(self.dict())

    def buildVars(self):
        res = {}

        dict = self.dict()
        dict['size_bytes'] = "0x%x" % self.size
        for k, v in dict.items():
            k = "STORAGE_DEVICE_%s_%s" % (self.name, k.upper())
            res[k] = v

        return res

    def type_str(self):
        return lookup_keyword(self.type, TYPES)

    def size_str(self):
        return size_format(self.size)

    def verify(self):
        if self.size == 0:
            raise ValidationError(self, "Size field is not set")
        if self.type is None:
            raise ValidationError(self, "Type field is not set")
        if self.name == '':
            raise ValidationError(self, "Name not specified")
        if len(self.name) > partition.PARTITION_NAME_SIZE:
            raise ValidationError(self, "Name too long, max. %u chars" % partition.PARTITION_NAME_SIZE)
