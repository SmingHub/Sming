import argparse, os, config, partition, configparser, string
from common import *
from config import *
import tkinter as tk
from tkinter import ttk, filedialog, messagebox, font
from collections import OrderedDict

app_name = 'Sming Hardware Profile Editor'

def read_property(obj, name):
    """Read an object property, preferring string representation
    """
    value = getattr(obj, name + '_str', None)
    return getattr(obj, name, None) if value is None else value()


def get_dict_value(dict, key, default):
    """Read dictionary value, creating one if it doesn't exist
    """
    if not key in dict:
        dict[key] = default
    return dict[key]


def load_config_vars(filename):
    out_base = os.environ['OUT_BASE']
    path = out_base + '/' + filename
    if not os.path.exists(path):
        return {}
    parser = configparser.ConfigParser()
    parser.optionxform = str # preserve case
    with open(path) as f:
        data = "[config]\n" + f.read()
    parser.read_string(data)
    return parser['config']


def checkProfilePath(filename):
    filename = os.path.realpath(filename)
    if filename.startswith(os.getcwd()):
        return True

    messagebox.showerror(
        'Invalid profile path',
        'Must be in working project directory where `make hwedit-config` was run')
    return False


def get_id(obj):
    """Get string identifier for a device or partition object
    """
    if isinstance(obj, partition.Entry):
        if obj.is_unused():
            return obj.device.name + '/' + str(obj.address)
    return obj.name

def resolve_id(config, id):
    """Get corresponding device or partition object given the ID value provided from get_id
    """
    elem = id.split('/')
    if len(elem) == 2:
        dev = config.devices.find_by_name(elem[0])
        return config.map().find_by_address(dev, elem[1])
    dev = config.devices.find_by_name(id)
    if dev is not None:
        return dev
    return config.map().find_by_name(id)

class Field:
    """Manages widget and associated variable
    """
    def __init__(self, var, widget):
        self.var = var
        self.widget = widget

    def get_value(self):
        return str(self.var.get())

    def is_disabled(self):
        try:
            return str(self.widget.cget('state')) == 'disabled'
        except Exception:
            return False


class EditState(dict):
    """Manage details of Config/Device/Partition editing using dictionary of Field objects
    """
    def __init__(self, editor, objectType, dictName, obj, enumDict):
        super().__init__(self)
        self.editor = editor
        self.objectType = objectType
        self.dictName = dictName
        self.enumDict = enumDict
        self.obj = obj
        self.name = obj.name
        self.schema = config.schema[objectType]
        self.allow_delete = False
        baseConfig = self.editor.getBaseConfig()
        optionBaseConfig = self.editor.getOptionBaseConfig()
        if objectType == 'Config':
            self.is_inherited = False
        elif objectType == 'Device':
            self.is_inherited = optionBaseConfig.devices.find_by_name(self.name) is not None
            self.allow_delete = not self.is_inherited
        elif objectType == 'Partition':
            if obj.is_unused():
                self.name = 'New Partition'
                self.is_inherited = False
            else:
                self.is_inherited = optionBaseConfig.map().find_by_name(self.name) is not None
                self.allow_delete = not self.is_inherited
        else:
            raise InputError('Unsupported objectType')

        if objectType == 'Config':
            self.base_obj = baseConfig
            self.obj_dict = editor.json
        else:
            self.base_obj = getattr(baseConfig, dictName).find_by_name(self.name)
            self.obj_dict = editor.json.get(dictName, {}).get(self.name, {})

        self.init()

    def init(self):
        f = self.editor.editFrame
        for c in f.winfo_children():
            c.destroy()

        label = ttk.Label(f, text=self.schema['title'], font='heading')
        label.pack()
        desc = self.schema.get('description')
        if desc is not None:
            label = ttk.Label(f, text=self.schema['description'])
            label.pack()

        f = self.controlFrame = ttk.Frame(self.editor.editFrame)
        f.pack(side=tk.TOP)
        self.array = {} # dictionary for array element variables
        self.row = 0
        keys = self.schema['properties'].keys()
        if not 'name' in keys:
            self.addControl('name')
        for k in keys:
            if k != 'devices' and k != 'partitions':
                self.addControl(k)
        f = ttk.Frame(self.editor.editFrame)
        f.pack(side=tk.BOTTOM)
        btn = ttk.Button(f, text='Apply', command=lambda *args: self.apply())
        btn.grid(row=0, column=0)
        btn = ttk.Button(f, text='Undo', command=lambda *args: self.init())
        btn.grid(row=0, column=1)
        if self.allow_delete:
            btn = ttk.Button(f, text='Delete', command=lambda *args: self.delete())
            btn.grid(row=0, column=2)
        self.editor.sizeEdit()


    def addControl(self, fieldName):
        schema = self.get_property(fieldName)
        fieldType = schema.get('type')
        frame = self.controlFrame
        disabled = False
        if fieldName == 'name':
            value = self.name
        else:
            value = self.obj_dict.get(fieldName, self.obj.dict().get(fieldName))
            if fieldName == 'device':
                disabled = True
            try:
                if fieldType == 'object':
                    value = '' if value is None else json.dumps(value)
                elif fieldType == 'array':
                    value = [] if value is None else json.dumps(value)
            except Exception as err:
                critical(str(err))
        var = tk.StringVar(value=value)

        if fieldType == 'boolean':
            c = ttk.Checkbutton(frame, text=fieldName, variable=var)
        else:
            l = tk.Label(frame, text=fieldName)
            l.grid(row=self.row, column=0, sticky=tk.W)
            values = self.enumDict.get(fieldName, schema.get('enum'))
            if values is None:
                c = tk.Entry(frame, width=64, textvariable=var)
            elif fieldType == 'array':
                c = ttk.Frame(frame)
                def array_changed(fieldName, key, var):
                    values = set(json_loads(var.get()))
                    if self.array[fieldName][key].get():
                        values.add(key)
                    else:
                        values.discard(key)
                    var.set(json.dumps(list(values)))
                elements = self.array[fieldName] = {}
                keys = list(values.keys())
                keys.sort()
                for k in keys:
                    v = values[k]
                    elements[k] = tk.BooleanVar(value=k in getattr(self.obj, fieldName))
                    btn = tk.Checkbutton(c, text = k + ': ' + v,
                        command=lambda *args, fieldName=fieldName, key=k, var=var: array_changed(fieldName, key, var),
                        variable=elements[k])
                    btn.grid(sticky=tk.W)
                    base = getattr(self.base_obj, fieldName)
                    if base is not None and k in base:
                        btn.configure(state='disabled')
            else:
                values.sort()
                c = ttk.Combobox(frame, values=values, textvariable=var)
                if fieldName == 'subtype':
                    def set_subtype_values():
                        t = self['type'].get_value()
                        t = partition.TYPES.get(t)
                        subtypes = list(partition.SUBTYPES.get(t, []))
                        subtypes.sort()
                        c.configure(values=subtypes)
                    c.configure(postcommand=set_subtype_values)
        self[fieldName] = Field(var, c)

        if fieldName == 'name':
            # Name is read-only for inherited devices/partitions
            if self.is_inherited:
                disabled = self.is_inherited
        # Internal 'partitions' are generally not editable, but make an exception to allow
        # creation of new partitions (on an 'unused' type) or changing the partition table offset
        if self.objectType == 'Partition' and self.is_inherited and self.obj.is_internal() and not self.obj.is_unused():
            if not (self.obj.is_internal(partition.INTERNAL_PARTITION_TABLE) and fieldName == 'address'):
                disabled = True
        if disabled:
            c.configure(state='disabled')
        c.grid(row=self.row, column=1, sticky=tk.EW)
        self.row += 1
        return c

    def apply(self):
        # Fetch base JSON for comparison
        baseConfig = self.editor.getBaseConfig()
        json_config = copy.deepcopy(self.editor.json)
        if self.objectType == 'Config':
            base = baseConfig
            json_dict = None
            obj = json_config
        else:
            base = getattr(baseConfig, self.dictName).find_by_name(self.name)
            json_dict = get_dict_value(json_config, self.dictName, {})
            obj = get_dict_value(json_dict, self.name, {})
            if self.objectType == 'Partition' and self.obj.is_unused():
                obj['device'] = self.obj.device.name
        base = {} if base is None else base.dict()
        new_name = None
        try:
            for k, f in self.items():
                if f.is_disabled():
                    continue
                value = f.get_value()
                schema = self.get_property(k)
                fieldType = schema.get('type')
                if k == 'name' and json_dict is not None:
                    value = value.strip()
                    if value != self.name:
                        if value in self.editor.config.map():
                            raise InputError("Name '%s' already used" % value)
                        old = json_dict.pop(self.name)
                        obj = json_dict[value] = old
                        new_name = value
                        # If renaming a device, then all partitions must be updated
                        if self.objectType == 'Device':
                            for n, p in json_config.get('partitions', {}).items():
                                if p.get('device') == self.name:
                                    p['device'] = new_name
                elif k == 'address' and self.objectType == 'Partition' and self.obj.is_internal(partition.INTERNAL_PARTITION_TABLE):
                    json_config['partition_table_offset'] = value
                elif value == '' and k != 'filename': # TODO mark 'allow empty' values in schema somehow
                    if k in obj:
                        del obj[k]
                elif fieldType == 'object' or fieldType == 'array':
                    obj[k] = {} if value == '' else json_loads(value)
                elif fieldType == 'boolean':
                    obj[k] = (value != '0')
                elif value.isdigit() and 'integer' in fieldType:
                    obj[k] = int(value)
                else:
                    obj[k] = value
                if k in base and obj.get(k) == base[k]:
                    del obj[k]

            if len(obj) == 0:
                del json_dict[self.name]
                if len(json_dict) == 0:
                    del json_config[self.dictName]


            if self.editor.verify_config(json_config):
                self.editor.set_json(json_config)
                if new_name is not None:
                    self.name = new_name
                if self.objectType != 'Config':
                    self.editor.selected = self.name
                self.editor.reload()
        except Exception as err:
            self.editor.user_error(err)
            raise err

    def delete(self):
        json_dict = self.editor.json[self.dictName]
        # If deleting a device, then delete all partitions first
        if self.objectType == 'Device':
            devparts = []
            partitions = self.editor.json.get('partitions', {})
            for n, p in partitions.items():
                if p.get('device') == self.name:
                    devparts.append(n)
            for n in devparts:
                partitions.pop(n)
            self.editor.selected = self.editor.config.devices[0].name
        else:
            self.editor.selected = self.obj.device.name
        del json_dict[self.name]
        if len(json_dict) == 0:
            del self.editor.json[self.dictName]
        self.editor.reload()

    def get_property(self, name):
        if name == 'name':
            return {'type': 'text'}
        else:
            return self.schema['properties'][name]

    def nameChanged(self):
        return self.name != self['name'].get_value()


class Rect:
    def __init__(self, x=0, y=0, width=0, height=0):
        self.x = x
        self.y = y
        self.x2 = x + width
        self.y2 = y + height

    def inflate(self, x, y):
        self.x -= x
        self.y -= y
        self.x2 += x
        self.y2 += y

    def getWidth(self):
        return self.x2 - self.x

    def setWidth(self, w):
        self.x2 = self.x + w

    def getHeight(self):
        return self.y2 - self.y

    def setHeight(self, h):
        self.y2 = self.y + h

    def pos(self, anchor=tk.NW):
        if anchor == tk.N:
            return ((self.x + self.x2) / 2, self.y)
        if anchor == tk.NE:
            return (self.x2, self.y)
        if anchor == tk.E:
            return (self.x2, (self.y + self.y2) / 2)
        if anchor == tk.SE:
            return (self.x2, self.y2)
        if anchor == tk.S:
            return ((self.x + self.x2) / 2, self.y2)
        if anchor == tk.SW:
            return (self.x, self.y2)
        if anchor == tk.W:
            return (self.x, (self.y + self.y2) / 2)
        if anchor == tk.CENTER:
            return ((self.x + self.x2) / 2, (self.y + self.y2) / 2)
        # Default and NW
        return (self.x, self.y)

    def bounds(self):
        return (self.x, self.y, self.x2, self.y2)


class TkMap(tk.Frame):
    def __init__(self, parent, editor):
        super().__init__(parent)
        self.pack(fill=tk.BOTH)
        self.editor = editor
        self.device = None
        self.selected_id = None
        self.bytesPerPixel = 32
        self.maxPartitionDrawSize = 0x4000
        canvas = self.canvas = tk.Canvas(self, height=150, xscrollincrement=1)
        canvas.pack(side=tk.TOP, expand=True, fill=tk.BOTH)
        self.autoZoom = tk.BooleanVar(value=True)
        btn = tk.Checkbutton(self, text='Auto Zoom', variable=self.autoZoom,
            command=lambda *args: self.update())
        btn.pack(side=tk.LEFT)
        s = ttk.Scrollbar(self, orient=tk.HORIZONTAL, command=canvas.xview)
        s.pack(side=tk.BOTTOM, fill=tk.X)
        canvas['xscrollcommand'] = s.set
        # Windows
        canvas.bind('<MouseWheel>', self.onMouseWheel)
        # Linux
        canvas.bind('<4>', self.onMouseWheel)
        canvas.bind('<5>', self.onMouseWheel)

    def clear(self):
        self.canvas.delete('all')
        self.items = {}

    def update(self):
        # Margins to separate drawn rectangular regions
        M = 5
        M_HEADING = 16
        M_OUTER = 20

        # Pixels to draw map row
        ROW_HEIGHT = 80
        # Text line spacing
        LINE_SPACE = 16
        # How far ticks extend below map
        TICK_LENGTH = 10

        self.clear()

        if self.device is None:
            return

        device = self.device
        canvas = self.canvas
        partitions = list(filter(lambda p: p.device == self.device, self.editor.config.map()))

        # Minimum width (in pixels) to draw a partition
        minPartitionWidth = M

        if self.autoZoom.get():
            self.bind('<Configure>', lambda x: self.update())
            maxPartitionDrawSize = 0xffffffffffffffff
            # Calculate draw width
            w = self.winfo_width() - (M_OUTER + M) * 2 - M * len(partitions)
            bytesPerPixel = self.device.size // w
            minSize = minPartitionWidth * bytesPerPixel
            # Now re-check to ensure all partitions have a sensible minimum drawn size
            def is_small(p):
                return p.size < minSize
            extra = sum(minSize - p.size for p in filter(is_small, partitions))
            bytesPerPixel = (self.device.size + extra) // w
        else:
            self.unbind('<Configure>')
            bytesPerPixel = self.bytesPerPixel
            maxPartitionDrawSize = self.maxPartitionDrawSize

        # Partitions smaller than this will be expanded
        minPartitionDrawSize = minPartitionWidth * bytesPerPixel

        labelFont = font.Font(family='fixed', size=8)
        labelFontBold = font.Font(family='fixed', size=8, weight='bold')
        headingFont = font.Font(family='courier', size=10, weight='bold')

        # Draw device title
        s = "[%s]" % device.name
        if device.size != 0:
            s += ': %s' % size_frac_str(device.size)
        if device.type != 0:
            s += ' (%s)' % device.type_str()
        canvas.create_text(self.winfo_width() / 2, 0, anchor=tk.N, text=s, font=headingFont, fill='blue')

        # When auto-zoom is disabled, drawing the map linearly is unhelpful,
        # it's too spread-out and navigation is difficult.
        # So, fix a limit for the drawn size of each partition (in bytes)
        # Marker ticks will be drawn according to this scale
        drawsize = 0      # Equivalent size (in bytes) for the partition
        x_device_end = 0  # Determines final x co-ordinate for end of device memory

        MIN_TICK_SPACING = 100
        def draw_tick(x, addr):
            canvas.create_line(x, r.y2 + M, x, r.y2 + M + TICK_LENGTH, fill='black', width=3)
            canvas.create_text(x, r.y + ROW_HEIGHT + M + TICK_LENGTH,
                anchor=tk.N,
                text = size_frac_str(addr),
                state='disabled',
                font=labelFontBold)

        # Track current partition area
        r_prev = Rect()
        r_prev.y = M_HEADING + M_OUTER
        r_prev.x = M_OUTER
        part_prev = None

        device_item = canvas.create_rectangle(r_prev.bounds(), fill='lightgray',  outline='black')
        canvas.tag_bind(device_item, '<Button-1>', lambda x: self.editor.editDevice(self.device))
        self.items[get_id(self.device)] = device_item

        for p in partitions:
            # Starting x co-ordinate for this partition depends on scale of previous partition
            r = copy.copy(r_prev)
            r.setHeight(ROW_HEIGHT)
            if part_prev is not None:
                r.x += M + drawsize * (p.address - part_prev.address) / part_prev.size / bytesPerPixel

            # Determine actual size to draw this partition (in bytes)
            drawsize = min(maxPartitionDrawSize, max(minPartitionDrawSize, p.size))
            r.setWidth(drawsize / bytesPerPixel)
            # Identify where end of device memory actually is in case partitions exceed this boundary
            if x_device_end == 0 and p.end() >= device.size - 1:
                sz = device.size - p.address
                x_device_end = r.x + (drawsize * sz / p.size / bytesPerPixel)

            # Draw tick marks
            div = device.size // 16
            addr = p.address - (p.address % div)
            x_next_tick = 0
            while addr <= p.end():
                if addr >= p.address:
                    x = r.x + (drawsize * (addr - p.address) / p.size / bytesPerPixel)
                    if x >= x_next_tick:
                        draw_tick(x, addr)
                        x_next_tick = x + MIN_TICK_SPACING
                addr += div

            # Outer rectangle for partition
            r2 = copy.copy(r)
            # r2.inflate(-M, -M)
            color = 'lightgray' if p.is_unused() else 'white'
            item = canvas.create_rectangle(r2.bounds(), fill=color, outline=color)
            canvas.tag_bind(item, "<Button-1>", lambda event, part=p: self.editor.editPartition(part))
            # Used region
            used = self.editor.get_used(p)
            if used.size != 0:
                r3 = copy.copy(r2)
                r3.inflate(-1, -1)
                if used.size > p.size:
                    color = 'lightpink'
                else:
                    r3.setWidth(used.size * r3.getWidth() / p.size)
                    color = 'lightblue'
                canvas.create_rectangle(r3.bounds(), fill=color, outline=color, state='disabled')
            item = canvas.create_rectangle(r2.bounds(), outline='blue', state='disabled')
            self.items[get_id(p)] = item

            # Internal text
            r2.inflate(-M, -M)
            def createText(r, anchor, text, font):
                if font.measure(text) < r.getWidth():
                    canvas.create_text(r.pos(tk.N if anchor == tk.CENTER else tk.NW), anchor=anchor, text=text, state='disabled', font=font)
            createText(r2, tk.NW, p.address_str(), labelFont)
            r2.y += LINE_SPACE
            r2.y += LINE_SPACE
            createText(r2, tk.CENTER, p.name, labelFontBold)
            r2.y += LINE_SPACE
            createText(r2, tk.CENTER, size_frac_str(p.size), labelFontBold)

            part_prev = p
            r_prev = r

        r2 = Rect(M_OUTER, M_HEADING + M_OUTER)
        r2.x2 = x_device_end
        r2.y2 = r_prev.y2
        r2.inflate(M, M)
        self.canvas.coords(device_item, r2.bounds())
        if x_device_end >= r_prev.x:
            draw_tick(x_device_end, device.size)
        self.scroll_width = r_prev.x2 + M_OUTER
        canvas.config(scrollregion=(0, 0, self.scroll_width, 0))

        # Highlight the selected item (if any)
        item = self.items.get(self.selected_id)
        if item is not None:
            self.canvas.itemconfigure(item, width=3)

    def onMouseWheel(self, event):
        if self.autoZoom.get():
            return
        shift = (event.state & 0x01) != 0
        control = (event.state & 0x04) != 0
        if event.num == 5:
            delta = -120
        elif event.num == 4:
            delta = 120
        else:
            delta = event.delta
        # Adjust draw size
        if shift and not control:
            ds = self.maxPartitionDrawSize
            if delta < 0:
                ds >>= 1
            else:
                ds <<= 1
            if ds >= 0x1000 and ds <= 0x100000:
                self.maxPartitionDrawSize = ds
                self.update()
        if control and not shift:
            # Zoom
            bpp = self.bytesPerPixel
            if delta < 0:
                bpp <<= 1
            else:
                bpp >>= 1
            if bpp >= 4 and bpp <= 0x10000:
                self.bytesPerPixel = bpp
                self.update()
        if not (shift or control):
            self.canvas.xview_scroll(delta, tk.UNITS)

    def set_device(self, dev):
        if self.device != dev:
            self.device = dev
            self.update()

    def select(self):
        id = self.editor.selected
        if id == self.selected_id:
            return
        item = self.items.get(self.selected_id)
        if item is not None:
            self.canvas.itemconfigure(item, width=1)
        item = self.items.get(id)
        if item is None:
            return
        self.canvas.itemconfigure(item, width=3)
        self.selected_id = id
        # Ensure item is visible
        pos = self.canvas.coords(item)
        id_x1 = pos[0] / self.scroll_width
        id_x2 = pos[2] / self.scroll_width
        xview = self.canvas.xview()
        if id_x1 > xview[0] and id_x1 < xview[1]:
            return
        if id_x2 > xview[0] and id_x2 < xview[1]:
            return
        if xview[0] >= id_x1 and xview[1] <= id_x2:
            return
        self.canvas.xview_moveto(id_x1)


class TkTree(tk.Frame):
    def __init__(self, parent, editor):
        super().__init__(parent)
        self.pack(fill=tk.BOTH)
        self.editor = editor
        s = ttk.Style()
        s.configure('Treeview', font='TkFixedFont')
        s.configure('Treeview.Heading', font='TkFixedFont', padding=4)
        self.init()

    def init(self):
        self.headings = [
            ("#0", "Partition"),
            ("type", "Type"),
            ("subtype", "Subtype"),
            ("start", "Start"),
            ("end", "End"),
            ("size", "Size"),
            ("used", "Used"),
            ("unused", "Unused"),
            ("filename", "Image Filename"),
        ]

        cols = [h[0] for h in self.headings[1:]]
        tree = self.tree = ttk.Treeview(self, selectmode=tk.BROWSE, columns=cols)
        tree.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)

        s = ttk.Scrollbar(self, orient=tk.VERTICAL, command=tree.yview)
        s.pack(side=tk.RIGHT, fill=tk.Y)
        tree['yscrollcommand'] = s.set

        for (k, v) in self.headings:
            tree.heading(k, text=v, anchor=tk.W)

        def select(*args):
            id = tree.focus()
            if id == '':
                return
            obj = resolve_id(self.editor.config, id)
            if isinstance(obj, partition.Entry):
                self.editor.editPartition(obj)
            elif isinstance(obj, storage.Device):
                self.editor.editDevice(obj)
        tree.bind('<<TreeviewSelect>>', select)

    def clear(self):
        for c in self.tree.get_children():
            self.tree.delete(c)

    def update(self):
        config = self.editor.config
        tree = self.tree
        fnt = font.Font(font='TkFixedFont')

        self.clear()

        columnWidths = [0] * len(self.headings)
        for i, (c, v) in enumerate(self.headings):
            columnWidths[i] = fnt.measure(v)

        def addItem(parent, id, values):
            text = []
            for i, (c, v) in enumerate(self.headings):
                v = values.get(c, '')
                if i != 0:
                    text.append(v)
                columnWidths[i] = max(columnWidths[i], fnt.measure(v))
            tree.insert(parent, 'end', id, open=True, text=values['#0'], values=text)

        # Devices are our root nodes
        map = config.map()
        for dev in config.devices:
            def is_used(p):
                return p.device == dev and not p.is_unused()
            used = sum(p.size for p in filter(is_used, map))
            values = {
                "#0": dev.name,
                "start": addr_format(0),
                "end": addr_format(dev.size - 1),
                "size": size_frac_str(dev.size),
                "used": size_frac_str(used),
                "unused": size_frac_str(dev.size - used),
                "type": dev.type_str()
            }
            addItem('', get_id(dev), values)

        # Partitions are children
        for p in config.map():
            used = self.editor.get_used(p)
            values = {
                "#0": p.name,
                "start": p.address_str(),
                "end": p.end_str(),
                "size": size_frac_str(p.size),
                "used": used.text,
                "type": p.type_str(),
                "subtype": p.subtype_str(),
                "filename": p.filename
            }
            if used.path != '':
                values['unused'] = size_frac_str(p.size - used.size)
            addItem(p.device.name, get_id(p), values)

        # Auto-size columns
        columnWidths[0] += 32 # Allow for indented child items
        for i, w in enumerate(columnWidths):
            w += 16
            tree.column(self.headings[i][0], stretch=False, width=w, minwidth=w)

        self.select()


    def select(self):
        id = self.editor.selected
        tree = self.tree
        if tree.exists(id) and tree.focus() != id:
            tree.focus(id)
            tree.selection_set(id)
            tree.see(id)


class Editor:
    def __init__(self, root):
        root.title(app_name)
        self.main = root
        self.edit = None
        self.selected = ''
        self.config_vars = load_config_vars('config.mk')
        self.config_vars.update(load_config_vars('debug.mk'))
        self.initialise()

    def initialise(self):
        self.main.option_add('*tearOff', False)

        hwFilter = [('Hardware Profiles', '*' + HW_EXT)]

        # Menus
        def fileNew():
            self.reset()
            self.reload()
            self.editDevice(self.config.devices[0])

        def fileOpen():
            filename = filedialog.askopenfilename(
                title='Select profile ' + HW_EXT + ' file',
                filetypes=hwFilter,
                initialdir=os.getcwd())
            if filename != '' and checkProfilePath(filename):
                self.loadConfig(filename)

        def fileSave():
            filename = self.json['name']
            filename = filedialog.asksaveasfilename(
                title='Save profile to file',
                filetypes=hwFilter,
                initialfile=filename,
                initialdir=os.getcwd())
            if filename != '' and checkProfilePath(filename):
                ext = os.path.splitext(filename)[1]
                if ext != HW_EXT:
                    filename += HW_EXT
                json_save(self.json, filename)

        # Toolbar
        toolbar = ttk.Frame(self.main)
        toolbar.pack(side=tk.TOP, fill=tk.X)
        btn = ttk.Button(toolbar, text="New", command=fileNew)
        btn.grid(row=0, column=1)
        btn = ttk.Button(toolbar, text="Open...", command=fileOpen)
        btn.grid(row=0, column=2)
        btn = ttk.Button(toolbar, text="Save...", command=fileSave)
        btn.grid(row=0, column=3)
        sep = ttk.Separator(toolbar, orient=tk.VERTICAL)
        sep.grid(row=0, column=4, sticky=tk.NS)
        btn = ttk.Button(toolbar, text='Edit Config', command=self.editConfig)
        btn.grid(row=0, column=5)
        btn = ttk.Button(toolbar, text='Add Device', command=self.addDevice)
        btn.grid(row=0, column=6)


        # Group main controls into areas which can be re-sized by the user
        pwin = ttk.PanedWindow(self.main, orient=tk.VERTICAL)
        pwin.pack(side=tk.TOP, expand=True, fill=tk.BOTH)

        self.map = TkMap(pwin, self)
        pwin.add(self.map)
        self.tree = TkTree(pwin, self)
        pwin.add(self.tree)


        # Lower pane
        lower = ttk.Frame(pwin)
        pwin.add(lower)

        # Edit frame
        frame = ttk.LabelFrame(lower, text='Edit')
        frame.pack(anchor=tk.NW, side=tk.LEFT, fill=tk.BOTH)
        canvas = self.editCanvas = tk.Canvas(frame, highlightthickness=0)
        canvas.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)
        self.editFrame = ttk.Frame(canvas)
        canvas.create_window(0, 0, window=self.editFrame, anchor=tk.NW)
        s = self.editScroll = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=canvas.yview)
        s.pack(side=tk.RIGHT, fill=tk.Y)
        canvas['yscrollcommand'] = s.set
        def configure(event):
            canvas.config(scrollregion=(0, 0, 0, self.editFrame.winfo_height()))
        self.editFrame.bind('<Configure>', configure)


        # JSON editor
        frame = self.jsonFrame = ttk.LabelFrame(lower, text='JSON')
        frame.pack(anchor=tk.NW, side=tk.LEFT, expand=True, fill=tk.BOTH)
        def apply(*args):
            try:
                json_config = json_loads(self.jsonEditor.get('1.0', 'end'))
                if self.verify_config(json_config):
                    self.set_json(json_config)
                    self.updateWindowTitle()
                    self.reload()
            except Exception as err:
                self.user_error(err)
        def undo(*args):
            self.jsonEditor.replace('1.0', 'end', to_json(self.json))
        f = ttk.Frame(frame)
        f.pack(anchor=tk.S, side=tk.BOTTOM)
        btn = ttk.Button(f, text="Apply", command=apply)
        btn.grid(row=0, column=0)
        btn = ttk.Button(f, text="Undo", command=undo)
        btn.grid(row=0, column=1)
        self.jsonEditor = tk.Text(frame, width=10, height=14)
        self.jsonEditor.pack(anchor=tk.N, side=tk.LEFT, expand=True, fill=tk.BOTH)
        s = ttk.Scrollbar(frame, orient=tk.VERTICAL, command=self.jsonEditor.yview)
        s.pack(anchor=tk.NE, side=tk.RIGHT, fill=tk.Y)
        self.jsonEditor['yscrollcommand'] = s.set

        # Status box
        self.status = tk.StringVar()
        status = ttk.Label(self.main, textvariable=self.status)
        status.pack(side=tk.BOTTOM, fill=tk.X)

        self.reset()

    def sizeEdit(self):
        self.main.update_idletasks()
        w = self.editFrame.winfo_width()
        self.editCanvas.config(width=w+8)

    def user_error(self, err):
        self.status.set(err)
        messagebox.showerror(type(err).__name__, err)

    def getBaseConfig(self):
        """Load the base configuration
        """
        return Config.from_json(self.json_base_config)
 
    def getOptionBaseConfig(self):
        """Load the base configuration with currently selected options applied
        """
        return Config.from_json(self.json_base_config, self.json.get('options', []))
 
    def loadConfig(self, filename):
        self.reset()
        # If this is a core profile, don't edit it but create a new profile based on it
        if filename.startswith(os.environ['SMING_HOME']):
            config_name = os.path.splitext(os.path.basename(filename))[0]
            self.json['base_config'] = config_name
        else:
            with open(filename) as f:
                json_config = json_loads(f.read())

        options = get_dict_value(self.json, 'options', [])
        for opt in os.environ.get('HWCONFIG_OPTS', '').replace(' ', '').split():
            if not opt in options:
                options.append(opt)

        self.set_json(json_config)

        self.reload()
        self.updateWindowTitle()
        self.editDevice(self.config.devices[0])

    def updateWindowTitle(self):
        name = self.json.get('name', None)
        if name is None:
            name = '(new)'
        else:
            name = '"' + name + '"'
        self.main.title(self.config.arch + ' ' + name + ' - ' + app_name)

    def verify_config(self, json_config):
        try:
            Config.from_json(json_config).verify(False)
            return True
        except Exception as err:
            self.user_error(err)
            return False

    def set_json(self, json_config):
        # Keep output order consistent
        self.json = OrderedDict()
        for k in config.schema['Config']['properties'].keys():
            if k in json_config:
                self.json[k] = json_config[k]

    def reset(self):
        self.tree.clear()
        self.map.clear()
        self.status.set('')
        self.json = OrderedDict()
        self.json['name'] = 'New Profile'
        self.json['base_config'] = 'standard'
        self.reload()
        self.updateWindowTitle()

    def resolve_path(self, path):
        tmp = str(path)
        while True:
            tmp = tmp.replace('(', '{')
            tmp = tmp.replace(')', '}')
            new_path = string.Template(tmp).substitute(self.config_vars)
            if new_path == tmp:
                return new_path
            tmp = new_path

    class Used:
        def __init__(self):
            self.text = ''
            self.size = 0
            self.path = ''

    def get_used(self, part):
        used = self.Used()
        if part.filename != '':
            try:
                used.path = self.resolve_path(part.filename)
                if os.path.exists(used.path):
                    used.size = os.path.getsize(used.path)
                    used.text = size_frac_str(used.size)
                else:
                    used.text = '(not found)'
            except KeyError as err:
                used.text = str(err) + ' undefined'
        return used

    def reload(self):
        with open(find_config(self.json['base_config'])) as f:
            self.json_base_config = json_loads(f.read())

        self.jsonEditor.replace('1.0', 'end', to_json(self.json))
        try:
            config = Config.from_json(self.json)
        except Exception as err:
            self.status.set(err)
            return

        self.status.set('')
        self.config = config

        self.map.update()
        self.tree.update()

    def select(self, obj):
        self.selected = get_id(obj)
        if isinstance(obj, partition.Entry):
            self.map.set_device(obj.device)
        else:
            self.map.set_device(obj)
        self.map.select()
        self.tree.select()

    def is_editing(self, obj):
        return getattr(self.edit, 'obj', None) == obj

    def editConfig(self):
        if self.is_editing(self.config):
            return
        # If we're editing a new device, for example, cancel it
        id = self.tree.tree.focus()
        if id != self.selected:
            obj = resolve_id(self.config, id)
            if obj is not None:
                self.select(obj)
        enumDict = {}
        enumDict['base_config'] = list(get_config_list().keys())
        optionlib = load_option_library()
        options = {}
        for k, v in optionlib.items():
            options[k] = v['description']
        enumDict['options'] = options
        self.edit = EditState(self, 'Config', None, self.config, enumDict)

    def addDevice(self):
        dev = storage.Device('New device')
        self.editDevice(dev)

    def editDevice(self, dev):
        if isinstance(dev, str):
            dev = self.config.devices.find_by_name(dev)
        if self.is_editing(dev):
            return
        self.select(dev)
        enumDict = {}
        enumDict['type'] = list((storage.TYPES).keys())
        self.edit = EditState(self, 'Device', 'devices', dev, enumDict)

    def editPartition(self, part):
        if isinstance(part, str):
            part = self.config.partitions.find_by_name(part)
        if self.is_editing(part):
            return
        self.select(part)
        enumDict = {}
        enumDict['type'] = list((partition.TYPES).keys() - ['storage', 'internal'])
        enumDict['subtype'] = []
        self.edit = EditState(self, 'Partition', 'partitions', part, enumDict)


def main():
    parser = argparse.ArgumentParser(description='Sming hardware profile editor')
    parser.add_argument('input', help='Name of existing hardware profile')
    args = parser.parse_args()

    root = tk.Tk()
    editor = Editor(root)
    editor.loadConfig(find_config(args.input))
    root.mainloop()


if __name__ == '__main__':
    try:
        print("TCL version %s" % tk.TclVersion)
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
