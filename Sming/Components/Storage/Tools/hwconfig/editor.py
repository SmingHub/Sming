import common, argparse, os, partition
from common import *
from config import *
import tkinter as tk
from tkinter import ttk, filedialog

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


class EditState(dict):
    """Manage details of device/partition editing using dictionary of tk.StringVar objects
    """
    def __init__(self, editor, objectType, dictName, obj):
        super().__init__(self)
        self.editor = editor
        self.type = objectType
        self.dictName = dictName
        self.name = obj.name
        self.schema = editor.schema['definitions'][objectType]
        self.json = get_dict_value(editor.json, dictName, {})
        self.obj = obj
        self.row = 0
        btn = ttk.Button(editor.editFrame, text="Apply", command=self.apply)
        btn.grid(row=100, column=0, columnspan=2)

    def addControl(self, fieldName, enumDict):
        frame = self.editor.editFrame
        schema = self.get_property(fieldName)
        value = read_property(self.obj, fieldName)
        if hasattr(value, 'name'):
            value = value.name
        elif value and schema['type'] == 'object':
            value = json.dumps(value)
        var = self[fieldName] = tk.StringVar(value=value)
        if schema['type'] == 'boolean':
            c = ttk.Checkbutton(frame, text=fieldName, variable=var)
        else:
            l = tk.Label(frame, text=fieldName)
            l.grid(row=self.row, column=0, sticky=tk.W)
            values = enumDict.get(fieldName, schema.get('enum'))
            if values is not None:
                c = ttk.Combobox(frame, values=values)
            else:
                c = tk.Entry(frame, width=64)
            c.configure(textvariable=var)
        c.configure(state=self.getState(fieldName))
        c.grid(row=self.row, column=1, sticky=tk.EW)
        self.row += 1
        return c

    def apply(self, *args):
        # Fetch base JSON for comparison
        baseConfig = copy.deepcopy(self.editor.baseConfig)
        baseConfig.parse_options(self.editor.json['options'])
        base = getattr(baseConfig, self.dictName).find_by_name(self.name)
        if base is None:
            base = {}
        else:
            base = base.dict()
        try:
            obj = get_dict_value(self.json, self.name, {})
            for k, v in self.items():
                value = v.get()
                schema = self.get_property(k)
                if k == 'name':
                    if value != self.name:
                        old = self.json.pop(self.name)
                        obj = self.json[value] = old
                        self.name = value
                        self.editor.updateEditTitle()
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
                del self.json[self.name]
            self.editor.reload()
        except AttributeError as err:
            self.editor.status.set(err)
        except ValueError as err:
            self.editor.status.set(err)


    def getState(self, field):
        # Name is read-only for inherited devices/partitions
        if field == 'name':
            objlist = getattr(self.editor.baseConfig, self.dictName)
            if objlist.find_by_name(self.name) is not None:
                return 'disabled'
        return 'normal'

    def get_property(self, name):
        if name == 'name':
            return {'type': 'text'}
        else:
            return self.schema['properties'][name]

    def keys(self):
        keys = list(self.schema['properties'].keys())
        keys.insert(0, 'name')
        return keys

    def nameChanged(self):
        return self.name != self['name'].get()


class Editor:
    def __init__(self, root):
        root.title(app_name)
        # Window resizing is focused around treeview @ (0, 0)
        root.columnconfigure(0, weight=1)
        root.rowconfigure(0, weight=1)
        root.option_add('*tearOff', False)
        self.main = root
        # ('winnative', 'clam', 'alt', 'default', 'classic', 'vista', 'xpnative')
        s = ttk.Style()
        # critical(str(s.theme_names()))
        # s.theme_use('xpnative')
        s.configure('Treeview', font='TkFixedFont')
        s.configure('Treeview.Heading', font='TkFixedFont')

    def initialise(self):
        with open(os.environ['HWCONFIG_SCHEMA']) as f:
            self.schema = json.load(f)

        # Menus
        def fileNew(*args):
            self.reset()
            self.reload()
        def fileOpen(*args):
            filename = filedialog.askopenfilename()
            if filename != '':
                self.loadConfig(filename)
        def fileSave(*args):
            filename = filedialog.asksaveasfilename()
            if filename != '':
                with open(filename, "w") as f:
                    json.dump(self.json, f, indent=4)
        menubar = tk.Menu(self.main)
        self.main['menu'] = menubar
        menu_file = tk.Menu(menubar)
        menubar.add_cascade(menu=menu_file, label='File')
        menu_file.add_command(label='New...', command=fileNew)
        menu_file.add_command(label='Open...', command=fileOpen)
        menu_file.add_command(label='Save...', command=fileSave)

        # Treeview for devices and partitions

        tree = ttk.Treeview(self.main, columns=['address', 'size', 'end', 'type', 'subtype', 'filename'])
        tree.grid(row=0, column=0, columnspan=3, sticky=tk.NSEW)
        self.tree = tree

        s = ttk.Scrollbar(self.main, orient=tk.VERTICAL, command=tree.yview)
        s.grid(row=0, column=4, sticky=tk.NS)
        tree['yscrollcommand'] = s.set

        tree.heading('address', text='Address')
        tree.heading('size', text='Size')
        tree.heading('end', text='End')
        tree.heading('type', text='Type')
        tree.heading('subtype', text='Sub-Type')
        tree.heading('filename', text='Image filename')

        # self.tree.tag_configure('device', font='+1')
        # tree.tag_configure('normal', font='+1')

        # Base configurations
        f = ttk.LabelFrame(self.main, text = 'Base Configuration')
        f.grid(row=1, column=0, sticky=tk.W)

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
        f.grid(row=2, column=0, sticky=tk.W)

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
        self.editFrame.grid(row=1, column=1, rowspan=2, sticky=tk.EW)

        # JSON editor
        jsonFrame = ttk.LabelFrame(self.main, text='JSON Configuration')
        jsonFrame.grid(row=1, column=2, rowspan=2, sticky=tk.EW)
        self.jsonEditor = tk.Text(jsonFrame, height=14, width=50)
        self.jsonEditor.grid(row=0, column=0, sticky=tk.NSEW)
        s = ttk.Scrollbar(jsonFrame, orient=tk.VERTICAL, command=self.jsonEditor.yview)
        s.grid(row=0, column=1, sticky=tk.NS)
        self.jsonEditor['yscrollcommand'] = s.set
        def apply(*args):
            self.json = json.loads(self.jsonEditor.get('1.0', 'end'))
            self.reload()
        btn = ttk.Button(jsonFrame, text="Apply", command=apply)
        btn.grid(row=1, column=0, columnspan=2)

        # Status box
        self.status = tk.StringVar()
        status = ttk.Label(self.main, textvariable=self.status)
        status.grid(row=3, column=0, columnspan=3, sticky=tk.EW)

        self.reset()


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
        self.baseConfig = Config.from_name(self.json['base_config'])
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
        self.json = {}
        self.json['base_config'] = 'standard'
        self.base_config.set('standard')
        for k, v in self.options.items():
            v.set(False)
        self.reload()
        self.updateWindowTitle()

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
            self.tree.insert('', 'end', dev.name, text=dev.name, open=True,
                tags = ['device'],
                values=[addr_format(0), dev.size_str(), addr_format(dev.size - 1), dev.type_str()])

        def get_part_id(part):
            if part.type == partition.INTERNAL_TYPE and part.subtype == partition.INTERNAL_UNUSED:
                id = part.device.name + '/' + part.address_str()
            else:
                id = part.name
            return id

        # Partitions are children
        for p in config.map():
            self.tree.insert(p.device.name, 'end', get_part_id(p), text=p.name,
                values=[p.address_str(), p.size_str(), p.end_str(), p.type_str(), p.subtype_str(), p.filename])

        # Base configuration
        self.base_config.set(config.base_config)

        # Options
        for k, v in self.options.items():
            v.set(k in config.options)

        if hasattr(self, 'edit'):
            id = self.edit.name
            if not self.tree.exists(id):
                id = get_part_id(self.edit.obj)
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
        self.editFrame.configure(text="Edit %s '%s'" % (self.edit.type, self.edit.name))

    def resetEditor(self):
        self.edit = None
        f = self.editFrame
        for c in f.winfo_children():
            c.destroy()
        return f

    def editDevice(self, dev):
        self.resetEditor()
        edit = self.edit = EditState(self, 'Device', 'devices', dev)
        self.updateEditTitle()

        values = {}
        values['type'] = list((storage.TYPES).keys())

        for k in edit.keys():
            edit.addControl(k, values)


    def editPartition(self, part):
        self.resetEditor()
        edit = self.edit = EditState(self, 'Partition', 'partitions', part)
        self.updateEditTitle()

        values = {}
        values['device'] = [dev.name for dev in self.config.devices]
        values['type'] = list((partition.TYPES).keys())
        subtypes = partition.SUBTYPES.get(part.type)
        if subtypes is not None:
            values['subtype'] = list(subtypes)

        for k in edit.keys():
            c = edit.addControl(k, values)
            if part.type == partition.INTERNAL_TYPE and part.subtype != partition.INTERNAL_UNUSED:
                c.configure(state='disabled')


def main():
    parser = argparse.ArgumentParser(description='Sming hardware profile editor')
    parser.add_argument('input', help='Name of existing hardware profile')
    args = parser.parse_args()

    root = tk.Tk()
    editor = Editor(root)
    editor.initialise()
    editor.loadConfig(find_config(args.input))
    root.mainloop()


if __name__ == '__main__':
    try:
        print("TCL version %s" % tk.TclVersion)
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
