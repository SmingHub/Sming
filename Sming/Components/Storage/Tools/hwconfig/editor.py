import argparse, os, partition, configparser, string
from common import *
from config import *
import tkinter as tk
from tkinter import ttk, filedialog, messagebox, font

app_name = 'Sming Hardware Profile Editor'

def read_property(obj, name):
    """Read an object property, preferring string representation
    """
    value = getattr(obj, name + '_str', None)
    return getattr(obj, name, '') if value is None else value()


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


def to_KB(value):
    """Round value up to whole KB
    """
    return (value + 1023) & ~0x3ff


def percent_used(used, total):
    """Get string showing size and percent of total
    """
    s = size_format(to_KB(used))
    if total != 0:
        s += " (%u%%)" % round(100 * used / total)
    return s


def checkProfilePath(filename):
    filename = os.path.realpath(filename)
    if filename.startswith(os.getcwd()):
        return True

    messagebox.showerror(
        'Invalid profile path',
        'Must be in working project directory where `make hwedit-config` was run')
    return False


class Field:
    """Manages widget and associated variable
    """
    def __init__(self, var, widget):
        self.var = var
        self.widget = widget

    def get_value(self):
        return str(self.var.get())

    def is_disabled(self):
        return str(self.widget.cget('state')) == 'disabled'


class EditState(dict):
    """Manage details of device/partition editing using dictionary of Field objects
    """
    def __init__(self, editor, objectType, dictName, obj, enumDict):
        super().__init__(self)
        self.editor = editor
        self.objectType = objectType
        self.dictName = dictName
        if objectType == 'Partition' and obj.is_unused():
            self.name = 'New Partition'
        else:
            self.name = obj.name
        self.schema = editor.schema['definitions'][objectType]
        self.obj = obj
        self.row = 0

        self.addControl('name')
        for k in self.schema['properties'].keys():
            self.addControl(k, enumDict)
        btn = ttk.Button(editor.editFrame, text="Apply", command=self.apply)
        btn.grid(row=100, column=0, columnspan=2)

    def addControl(self, fieldName, enumDict = {}):
        frame = self.editor.editFrame
        schema = self.get_property(fieldName)
        disabled = False
        value = read_property(self.obj, fieldName)
        if hasattr(value, 'name'):
            value = value.name
            disabled = True
        elif value and schema['type'] == 'object':
            value = json.dumps(value)
        var = tk.StringVar(value=value)
        if schema['type'] == 'boolean':
            c = ttk.Checkbutton(frame, text=fieldName, variable=var)
        else:
            l = tk.Label(frame, text=fieldName)
            l.grid(row=self.row, column=0, sticky=tk.W)
            values = enumDict.get(fieldName, schema.get('enum'))
            if values is not None:
                c = ttk.Combobox(frame, values=values)
                if fieldName == 'subtype':
                    def set_subtype_values():
                        t = self['type'].get_value()
                        t = partition.TYPES[t]
                        values = partition.SUBTYPES.get(t, [])
                        critical("t = %s, %s" % (t, values))
                        c.configure(values=list(values))
                    c.configure(postcommand=set_subtype_values)
            else:
                c = tk.Entry(frame, width=64)
            c.configure(textvariable=var)
        self[fieldName] = Field(var, c)

        if fieldName == 'name':
            # Name is read-only for inherited devices/partitions
            objlist = getattr(self.editor.getBaseConfig(), self.dictName)
            disabled = objlist.find_by_name(self.name) is not None
            var.set(self.name)
        # Internal 'partitions' are generally not editable, but make an exception to allow
        # creation of new partitions (on an 'unused' type) or changing the partition table offset
        if self.objectType == 'Partition':
            if self.obj.is_internal() and not self.obj.is_unused():
                if not (self.obj.is_internal(partition.INTERNAL_PARTITION_TABLE) and fieldName == 'address'):
                    disabled = True
        if disabled:
            c.configure(state='disabled')
        c.grid(row=self.row, column=1, sticky=tk.EW)
        self.row += 1
        return c

    def apply(self, *args):
        # Fetch base JSON for comparison
        baseConfig = self.editor.getBaseConfig()
        base = getattr(baseConfig, self.dictName).find_by_name(self.name)
        if base is None:
            base = {}
        else:
            base = base.dict()
        json_config = copy.deepcopy(self.editor.json)
        json_object = get_dict_value(json_config, self.dictName, {})
        new_name = None
        try:
            obj = get_dict_value(json_object, self.name, {})
            if self.objectType == 'Partition' and self.obj.is_unused():
                obj['device'] = self.obj.device.name
            for k, f in self.items():
                if f.is_disabled():
                    continue
                value = f.get_value()
                schema = self.get_property(k)
                if k == 'name':
                    value = value.strip()
                    if value != self.name:
                        if value in self.editor.config.map():
                            self.editor.status.set("Name '%s' already used" % value)
                            return
                        old = json_object.pop(self.name)
                        obj = json_object[value] = old
                        new_name = value
                        # If renaming a device, then all partitions must be updated
                        if self.objectType == 'Device':
                            for n, p in json_config.get('partitions', {}).items():
                                if p['device'] == self.name:
                                    p['device'] = new_name
                elif k == 'address' and self.objectType == 'Partition' and self.obj.is_internal(partition.INTERNAL_PARTITION_TABLE):
                    if parse_int(value) == baseConfig.partitions.offset:
                        if 'partition_table_offset' in json_config:
                            del json_config['partition_table_offset']
                    else:
                        json_config['partition_table_offset'] = value
                elif value == '' and k != 'filename': # TODO mark 'allow empty' values in schema somehow
                    if k in obj:
                        del obj[k]
                elif schema['type'] == 'object':
                    obj[k] = {} if value == '' else json.loads(value)
                elif schema['type'] == 'boolean':
                    obj[k] = (value != '0')
                elif value.isdigit() and 'integer' in schema['type']:
                    obj[k] = int(value)
                else:
                    obj[k] = value
                if k in base and obj[k] == base[k]:
                    del obj[k]
            if len(obj) == 0:
                del json_object[self.name]
            Config.from_json(json_config).verify(False)
            self.editor.json = json_config
            if new_name is not None:
                self.name = new_name
                self.editor.updateEditTitle()
            self.editor.reload()
        except InputError as err:
            self.editor.user_error(err)
        except AttributeError as err:
            self.editor.user_error(err)
        except ValueError as err:
            self.editor.user_error(err)


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

    def pos(self):
        return (self.x, self.y)

    def bounds(self):
        return (self.x, self.y, self.x2, self.y2)


class TkMap(tk.Frame):
    def __init__(self, parent, editor):
        super().__init__(parent, width=200, height=200)
        self.pack(fill=tk.BOTH)
        self.editor = editor
        canvas = self.canvas = tk.Canvas(self, width=200, height=200)
        canvas.pack(side=tk.TOP, expand=True, fill=tk.BOTH)
        s = ttk.Scrollbar(self, orient=tk.HORIZONTAL, command=canvas.xview)
        s.pack(side=tk.BOTTOM, fill=tk.X)
        canvas['xscrollcommand'] = s.set

    def update(self, device):
        canvas = self.canvas
        canvas.delete('all')

        labelFont = font.Font(family='fixed', size=8)
        labelFontBold = font.Font(family='fixed', size=8, weight='bold')

        # Margins to separate drawn rectangular regions
        M = 5
        M_OUTER = 20

        # Pixels to draw map row
        ROW_HEIGHT = 160
        # Text line spacing
        LINE_SPACE = 16
        # How far ticks extend below map
        TICK_LENGTH = 10
        # Linear scaling for view
        BYTES_PER_PIXEL = 32
        def xs(x):
            return round(x / BYTES_PER_PIXEL)

        # Drawing the map linearly would make navigation very difficult, too spread-out
        # Instead, fix a limit for the drawn size of each partition (in bytes)
        # Marker ticks will be drawn according to this scale
        MAX_DRAWSIZE = 16 * 1024
        drawsize = 0      # Equivalent size (in bytes) for the partition
        x_device_end = 0  # Determines final x co-ordinate for end of device memory

        def draw_tick(x, addr):
            canvas.create_line(x, r.y, x, r.y2 + 10, fill='black', width=3)
            canvas.create_text(x, r.y + ROW_HEIGHT + TICK_LENGTH,
                anchor=tk.N,
                text=str(addr / 1024 / 1024) + 'MB',
                state='disabled',
                font=labelFontBold)

        # Track current partition area
        r_prev = Rect()
        r_prev.y = M_OUTER
        r_prev.x = M_OUTER
        part_prev = None

        class Used:
            def __init__(self):
                self.text = ''
                self.size = 0
                self.path = ''

        for p in self.editor.config.map():
            if p.device != device:
                continue

            used = Used()
            if p.filename != '':
                try:
                    used.path = self.editor.resolve_path(p.filename)
                    if os.path.exists(used.path):
                        used.size = os.path.getsize(used.path)
                        used.text = percent_used(used.size, p.size)
                    else:
                        used.text = '(not found)'
                except KeyError as err:
                    used.text = str(err) + ' undefined'

            # Starting x co-ordinate for this partition depends on scale of previous partition
            r = copy.copy(r_prev)
            r.setHeight(ROW_HEIGHT)
            if part_prev is not None:
                r.x += xs(drawsize * (p.address - part_prev.address) / part_prev.size)

            # Determine actual size to draw this partition (in bytes)
            drawsize = min(MAX_DRAWSIZE, p.size)
            r.setWidth(xs(drawsize))
            # Identify where end of device memory actually is in case partitions exceed this boundary
            if x_device_end == 0 and p.end() >= device.size - 1:
                sz = device.size - p.address
                x_device_end = r.x + xs(drawsize * sz / p.size)

            # Draw tick marks
            div = 256 * 1024
            addr = p.address - (p.address % div)
            while addr <= p.end():
                if addr >= p.address:
                    x = r.x + xs(drawsize * (addr - p.address) / p.size)
                    draw_tick(x, addr)
                addr += div

            r2 = copy.copy(r)
            r2.inflate(-M, -M)
            id = canvas.create_rectangle(r2.bounds(), fill='lightgray' if p.is_unused() else 'gray', activefill='white', outline='red')
            canvas.tag_bind(id, "<Button-1>", lambda event, part=p: self.editor.editPartition(part))
            r2.inflate(-M, -M)
            canvas.create_text(r2.pos(), anchor=tk.NW, text=p.address_str(), state='disabled', font=labelFont)
            r2.y += LINE_SPACE
            canvas.create_text(r2.pos(), anchor=tk.NW, text=p.name, state='disabled', font=labelFontBold)
            r2.y += LINE_SPACE
            canvas.create_text(r2.pos(), anchor=tk.NW, text=p.size_str(), state='disabled', font=labelFontBold)
            if not p.is_internal():
                r2.y += LINE_SPACE
                canvas.create_text(r2.pos(), anchor=tk.NW, text=p.type_str() + ' / ' + p.subtype_str(), state='disabled', font=labelFont)
            r2.y += LINE_SPACE
            if used.size != 0:
                r2.setWidth(used.size * r2.getWidth() / p.size)
                canvas.create_rectangle(r2.bounds(), fill='lightblue', outline='lightblue', state='disabled')
            r2.x += M
            r2.y += LINE_SPACE
            canvas.create_text(r2.pos(), anchor=tk.NW, text=used.text, state='disabled', font=labelFont)
            if p.filename != '':
                r2.y += LINE_SPACE
                canvas.create_text(r2.pos(), anchor=tk.NW, text=p.filename, state='disabled', font=labelFont)
                if used.path != p.filename:
                    r2.y += LINE_SPACE
                    canvas.create_text(r2.pos(), anchor=tk.NW, text=used.path, state='disabled', font=labelFont)

            part_prev = p
            r_prev = r

        r2 = Rect(M_OUTER, M_OUTER)
        r2.x2 = x_device_end
        r2.y2 = r.y2
        canvas.create_rectangle(r2.bounds(), outline='black', width=3, state='disabled')
        if x_device_end >= r.x:
            draw_tick(x_device_end, device.size)
        canvas.config(scrollregion=(0, 0, r.x2 + 100, 0))



class Editor:
    def __init__(self, root):
        root.title(app_name)
        self.main = root
        self.edit = None
        self.config_vars = load_config_vars('config.mk')
        self.config_vars.update(load_config_vars('debug.mk'))
        self.initialise()

    def initialise(self):
        self.main.option_add('*tearOff', False)
        s = ttk.Style()
        s.configure('Treeview', font='TkFixedFont')
        s.configure('Treeview.Heading', font='TkFixedFont')

        with open(os.environ['HWCONFIG_SCHEMA']) as f:
            self.schema = json.load(f)

        hwFilter = [('Hardware Profiles', '*' + HW_EXT)]

        # Menus
        def fileNew(*args):
            self.reset()
            self.reload()

        def fileOpen(*args):
            filename = filedialog.askopenfilename(
                title='Select profile ' + HW_EXT + ' file',
                filetypes=hwFilter,
                initialdir=os.getcwd())
            if filename != '' and checkProfilePath(filename):
                self.loadConfig(filename)

        def fileSave(*args):
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
                with open(filename, "w") as f:
                    json.dump(self.json, f, indent=4)

        def editAddDevice(*args):
            dev = storage.Device('New device')
            self.editDevice(dev)

        menubar = tk.Menu(self.main)
        self.main['menu'] = menubar
        menu_file = tk.Menu(menubar)
        menubar.add_cascade(menu=menu_file, label='File')
        menu_file.add_command(label='New...', command=fileNew)
        menu_file.add_command(label='Open...', command=fileOpen)
        menu_file.add_command(label='Save...', command=fileSave)
        menu_edit = tk.Menu(menubar)
        menubar.add_cascade(menu=menu_edit, label='Edit')
        menu_edit.add_command(label='Add Device', command=editAddDevice)

        # Toolbar
        toolbar = ttk.Frame(self.main)
        toolbar.pack(side=tk.TOP, fill=tk.X)
        btnNew = ttk.Button(toolbar, text="New", command=fileNew)
        btnNew.grid(row=0, column=1)
        btnOpen = ttk.Button(toolbar, text="Open...", command=fileOpen)
        btnOpen.grid(row=0, column=2)
        btnSave = ttk.Button(toolbar, text="Save...", command=fileSave)
        btnSave.grid(row=0, column=3)

        # Group controls into two areas (top and bottom) which are sizeable by the user
        pwin = ttk.PanedWindow(self.main, orient=tk.VERTICAL)
        pwin.pack(side=tk.TOP, expand=True, fill=tk.BOTH)

        # Place alternate views in a tabbed area
        self.notebook = ttk.Notebook(pwin)
        pwin.add(self.notebook)

        # Treeview for devices and partitions

        f = ttk.Frame(self.notebook)
        self.notebook.add(f, text="Tree")
        tree = self.tree = ttk.Treeview(f, columns=['start', 'end', 'size', 'used', 'type', 'subtype', 'filename'])
        tree.pack(side=tk.LEFT, expand=True, fill=tk.BOTH)

        s = ttk.Scrollbar(f, orient=tk.VERTICAL, command=tree.yview)
        s.pack(side=tk.RIGHT, fill=tk.Y)
        tree['yscrollcommand'] = s.set

        tree.heading('start', text='Start', anchor=tk.W)
        tree.heading('end', text='End', anchor=tk.W)
        tree.heading('size', text='Size', anchor=tk.W)
        tree.heading('used', text='Used', anchor=tk.W)
        tree.heading('type', text='Type', anchor=tk.W)
        tree.heading('subtype', text='Sub-Type', anchor=tk.W)
        tree.heading('filename', text='Image filename', anchor=tk.W)

        def select(*args):
            id = tree.focus()
            if id == '':
                return
            item = tree.item(id)
            if 'device' in item['tags']:
                dev = self.device_from_id(id)
                self.editDevice(dev)
            else:
                part = self.config.map().find_by_name(id)
                if part is None:
                    dev = self.device_from_id(tree.parent(id))
                    addr = item['values'][0]
                    part = self.config.map().find_by_address(dev, addr)
                self.editPartition(part)
        tree.bind('<<TreeviewSelect>>', select)

        # map of spiFlash device
        self.map = TkMap(self.notebook, self)
        self.notebook.add(self.map, text = 'map')

        # Lower frame
        frame = ttk.Frame(pwin)
        frame.pack(expand=True, fill=tk.BOTH)
        pwin.add(frame)

        frame.rowconfigure(0, weight=1)
        frame.columnconfigure(0, weight=1)

        # Base configurations
        f = ttk.LabelFrame(frame, text = 'Base Configuration')
        f.grid(row=0, column=0, sticky=tk.SW)

        def base_config_changed(*args):
            self.json['base_config'] = self.base_config.get()
            self.reload()

        self.base_config = tk.StringVar(value = 'standard')
        config_list = ttk.Combobox(f,
            textvariable = self.base_config,
            values = list(get_config_list().keys()))
        config_list.bind('<<ComboboxSelected>>', base_config_changed)
        config_list.grid()

        # Option checkboxes

        f = ttk.LabelFrame(frame, text = 'Options')
        f.grid(row=1, column=0, sticky=tk.NW)

        def options_changed(*args):
            self.json['options'] = []
            for k, v in self.options.items():
                if v.get():
                    self.json['options'].append(k)
            self.reload()

        self.options = {}
        for k, v in load_option_library().items():
            self.options[k] = tk.BooleanVar()
            btn = tk.Checkbutton(f, text = k + ': ' + v['description'],
                command=options_changed, variable=self.options[k])
            btn.grid(sticky=tk.W)

        # Edit frame
        self.editFrame = ttk.LabelFrame(frame, text='Edit Object')
        self.editFrame.grid(row=0, column=1, rowspan=2, sticky=tk.SW)

        # JSON editor
        jsonFrame = ttk.LabelFrame(frame, text='JSON Configuration')
        jsonFrame.grid(row=0, column=2, rowspan=2, sticky=tk.NS)
        def apply(*args):
            try:
                json_config = json.loads(self.jsonEditor.get('1.0', 'end'))
                Config.from_json(json_config).verify(False)
                self.json = json_config
                self.updateWindowTitle()
                self.reload()
            except InputError as err:
                self.user_error(err)
            except AttributeError as err:
                self.user_error(err)
            except ValueError as err:
                self.user_error(err)
        btn = ttk.Button(jsonFrame, text="Apply", command=apply)
        btn.pack(anchor=tk.S, side=tk.BOTTOM)
        self.jsonEditor = tk.Text(jsonFrame, height=14)
        self.jsonEditor.pack(anchor=tk.N, side=tk.LEFT, fill=tk.BOTH)
        s = ttk.Scrollbar(jsonFrame, orient=tk.VERTICAL, command=self.jsonEditor.yview)
        s.pack(anchor=tk.N, side=tk.RIGHT, fill=tk.Y)
        self.jsonEditor['yscrollcommand'] = s.set

        # Status box
        self.status = tk.StringVar()
        status = ttk.Label(self.main, textvariable=self.status)
        status.pack(side=tk.BOTTOM, fill=tk.X)

        self.reset()

    def user_error(self, err):
        self.status.set(err)
        messagebox.showerror(type(err).__name__, err)

    def getBaseConfig(self):
        """Load the base configuration with currently selected options applied
        """
        return Config.from_json(self.json_base_config, self.json.get('options', []))
 
    def loadConfig(self, filename):
        self.reset()
        # If this is a core profile, don't edit it but create a new profile based on it
        if filename.startswith(os.environ['SMING_HOME']):
            self.json = {}
            self.json['name'] = 'New profile'
            config_name = os.path.splitext(os.path.basename(filename))[0]
            self.json['base_config'] = config_name
        else:
            with open(filename) as f:
                self.json = json.loads(jsmin(f.read()))

        with open(find_config(self.json['base_config'])) as f:
            self.json_base_config = json.loads(jsmin(f.read()))

        options = get_dict_value(self.json, 'options', [])
        for opt in os.environ.get('HWCONFIG_OPTS', '').replace(' ', '').split():
            if not opt in options:
                options.append(opt)

        self.reload()
        self.updateWindowTitle()

    def updateWindowTitle(self):
        name = self.json.get('name', None)
        if name is None:
            name = '(new)'
        else:
            name = '"' + name + '"'
        self.main.title(self.config.arch + ' ' + name + ' - ' + app_name)

    def clear(self):
        # TODO: Prompt to save changes

        # Clear the tree, etc.
        for c in self.tree.get_children():
            self.tree.delete(c)
        self.status.set('')

    def reset(self):
        self.clear()
        self.resetEditor()
        self.json = {"name": "New Profile"}
        self.json['base_config'] = 'standard'
        self.base_config.set('standard')
        for k, v in self.options.items():
            v.set(False)
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

    def reload(self):
        self.clear()
        self.jsonEditor.replace('1.0', 'end', to_json(self.json))
        try:
            config = self.config = Config.from_json(self.json)
        except InputError as err:
            self.status.set(str(err))
            return

        self.map.update(config.devices[0])

        # Devices are our root nodes
        for dev in config.devices:
            used = 0
            for p in config.map():
                if p.device == dev and not p.is_unused():
                    used += p.size
            self.tree.insert('', 'end', dev.name, text=dev.name, open=True,
                tags = ['device'],
                values=[addr_format(0), addr_format(dev.size - 1), dev.size_str(), percent_used(used, dev.size), dev.type_str()])

        # Partitions are children
        for p in config.map():
            if p.is_unused():
                id = p.device.name + '/' + p.address_str()
            else:
                id = p.name

            def get_used():
                if p.filename == '':
                    return ''
                try:
                    path = self.resolve_path(p.filename)
                except KeyError as err:
                    return str(err) + ' undefined'
                if not os.path.exists(path):
                    return '(not found)'
                return percent_used(os.path.getsize(path), p.size)

            self.tree.insert(p.device.name, 'end', id, text=p.name,
                values=[p.address_str(), p.end_str(), p.size_str(), get_used(), p.type_str(), p.subtype_str(), p.filename])

        # Base configuration
        self.base_config.set(config.base_config)

        # Options
        for k, v in self.options.items():
            v.set(k in config.options)

        if self.edit is not None:
            id = self.edit.name
            if self.tree.exists(id):
                self.tree.focus(id)
                self.tree.selection_set(id)


    def device_from_id(self, id):
        item = self.tree.item(id)
        if 'device' in item['tags']:
            return self.config.devices.find_by_name(item['text'])
        else:
            return None

    def updateEditTitle(self):
        self.editFrame.configure(text="Edit %s '%s'" % (self.edit.objectType, self.edit.name))

    def resetEditor(self):
        self.edit = None
        f = self.editFrame
        for c in f.winfo_children():
            c.destroy()
        self.editFrame.configure(text='')
        return f

    def editDevice(self, dev):
        enumDict = {}
        enumDict['type'] = list((storage.TYPES).keys())
        self.resetEditor()
        self.edit = EditState(self, 'Device', 'devices', dev, enumDict)
        self.updateEditTitle()


    def editPartition(self, part):
        enumDict = {}
        enumDict['device'] = [dev.name for dev in self.config.devices]
        enumDict['type'] = list((partition.TYPES).keys() - ['storage', 'internal'])
        enumDict['subtype'] = []
        self.resetEditor()
        self.edit = EditState(self, 'Partition', 'partitions', part, enumDict)
        self.updateEditTitle()


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
