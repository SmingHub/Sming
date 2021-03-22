import argparse, os, partition, configparser, string
from common import *
from config import *
import tkinter as tk
from tkinter import ttk, filedialog, messagebox

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


class Editor:
    def __init__(self, root):
        root.title(app_name)
        self.main = root
        self.edit = None
        self.config_vars = load_config_vars('config.mk')
        self.config_vars.update(load_config_vars('debug.mk'))
        self.initialise()

    def initialise(self):
        # Window resizing is focused around treeview @ (0, 0)
        self.main.columnconfigure(0, weight=1)
        self.main.rowconfigure(1, weight=1)
        self.main.rowconfigure(2, weight=3)
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
        toolbar.grid(row=0, column=0, columnspan=4, sticky=tk.W)
        btnNew = ttk.Button(toolbar, text="New", command=fileNew)
        btnNew.grid(row=0, column=1)
        btnOpen = ttk.Button(toolbar, text="Open...", command=fileOpen)
        btnOpen.grid(row=0, column=2)
        btnSave = ttk.Button(toolbar, text="Save...", command=fileSave)
        btnSave.grid(row=0, column=3)

        # Treeview for devices and partitions

        tree = ttk.Treeview(self.main, columns=['start', 'end', 'size', 'used', 'type', 'subtype', 'filename'])
        tree.grid(row=1, column=0, columnspan=3, sticky=tk.NSEW)
        self.tree = tree

        s = ttk.Scrollbar(self.main, orient=tk.VERTICAL, command=tree.yview)
        s.grid(row=1, column=4, sticky=tk.NS)
        tree['yscrollcommand'] = s.set

        tree.heading('start', text='Start', anchor=tk.W)
        tree.heading('end', text='End', anchor=tk.W)
        tree.heading('size', text='Size', anchor=tk.W)
        tree.heading('used', text='Used', anchor=tk.W)
        tree.heading('type', text='Type', anchor=tk.W)
        tree.heading('subtype', text='Sub-Type', anchor=tk.W)
        tree.heading('filename', text='Image filename', anchor=tk.W)

        # Base configurations
        f = ttk.LabelFrame(self.main, text = 'Base Configuration')
        f.grid(row=2, column=0, sticky=tk.SW)

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

        f = ttk.LabelFrame(self.main, text = 'Options')
        f.grid(row=3, column=0, sticky=tk.NW)

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

        # Selection handling
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

        # Edit frame
        self.editFrame = ttk.LabelFrame(self.main, text='Edit Object')
        self.editFrame.grid(row=2, column=1, rowspan=2, sticky=tk.SW)

        # JSON editor
        jsonFrame = ttk.LabelFrame(self.main, text='JSON Configuration')
        jsonFrame.grid(row=2, column=2, rowspan=2, sticky=tk.NS)
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
        status.grid(row=4, column=0, columnspan=3, sticky=tk.EW)

        self.reset()

    def user_error(self, err):
        self.status.set(err)
        messagebox.showerror(type(err).__name__, err)

    def getBaseConfig(self):
        """Load the base configuration with currently selected options applied
        """
        return Config.from_json(self.json_base_config, self.json['options'])
 
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
