import common, argparse, os, partition
from common import *
from config import *
import tkinter as tk
from tkinter import ttk

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
    """Dictionary of tk.StringVar objects
    """
    def __init__(self, editor, objectType, dictName, objectName):
        super().__init__(self)
        self.editor = editor
        self.type = objectType
        self.dictName = dictName
        self.name = objectName
        self.schema = editor.schema['definitions'][objectType]
        self.json = get_dict_value(editor.json, dictName, {})
        self.config = getattr(editor.config, dictName)
        self.obj = self.config.find_by_name(objectName)

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

    def initialise(self):
        with open(os.environ['HWCONFIG_SCHEMA']) as f:
            self.schema = json.load(f)

        # Menus
        def newFile(*args):
            critical('newFile')
            self.reset()
            self.reload()
        def openFile(*args):
            critical('openFile')
        def saveFile(*args):
            critical('saveFile')
        menubar = tk.Menu(self.main)
        self.main['menu'] = menubar
        menu_file = tk.Menu(menubar)
        menubar.add_cascade(menu=menu_file, label='File')
        menu_file.add_command(label='New...', command=newFile)
        menu_file.add_command(label='Open...', command=openFile)
        menu_file.add_command(label='Save...', command=saveFile)

        # Treeview for devices and partitions

        tree = ttk.Treeview(self.main, columns=['address', 'size', 'type', 'subtype', 'filename'])
        tree.grid(row=0, column=0, columnspan=2, sticky=tk.NSEW)
        self.tree = tree

        s = ttk.Scrollbar(self.main, orient=tk.VERTICAL, command=tree.yview)
        s.grid(row=0, column=2, sticky=tk.NS)
        tree['yscrollcommand'] = s.set

        tree.heading('address', text='Address')
        tree.heading('size', text='Size')
        tree.heading('type', text='Type')
        tree.heading('subtype', text='Sub-Type')
        tree.heading('filename', text='Image filename')

        self.tree.tag_configure('device', font='+1')
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
            item = tree.item(id)
            if 'device' in item['tags']:
                dev = self.device_from_id(id)
                s = 'Device: ' + dev.to_json()
                self.editDevice(dev)
            else:
                dev = self.device_from_id(tree.parent(id))
                addr = item['values'][0]
                part = self.config.map().find_by_address(dev, addr)
                s = 'Partition: ' + part.to_json()
                self.editPartition(part)
        tree.bind('<<TreeviewSelect>>', select)

        # Edit frame
        self.editFrame = ttk.LabelFrame(self.main, text='Edit Object')
        self.editFrame.grid(row=1, column=1, rowspan=2, sticky=tk.EW)

        # Status box
        self.status = tk.StringVar()
        status = ttk.Label(self.main, textvariable=self.status)
        status.grid(row=3, column=0, columnspan=3, sticky=tk.EW)

        self.reset()


    def loadConfig(self, config_name):
        self.reset()
        filename = find_config(config_name)
        with open(filename) as f:
            self.json = json.loads(jsmin(f.read()))
        if not hasattr(self.json, 'options'):
            self.json['options'] = []
        options = self.json['options']
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
        try:
            self.config = Config.from_json(self.json)
        except InputError as err:
            self.status.set(str(err))
            return

        config = self.config

        # Devices are our root nodes
        for dev in config.devices:
            self.tree.insert('', 'end', dev.name, text=dev.name, open=True,
                tags = ['device'],
                values=['', dev.size_str(), dev.type_str()])

        # Partitions are children
        for p in config.map():
            self.tree.insert(p.device.name, 'end', text=p.name if p.name != '(unused)' else '',
                tags = ['unused' if p.type == 0xff else 'normal'],
                values=[p.address_str(), p.size_str(), p.type_str(), p.subtype_str(), p.filename])

        # Base configuration
        self.base_config.set(config.base_config)

        # Options
        for k, v in self.options.items():
            v.set(k in config.options)


    def device_from_id(self, id):
        item = self.tree.item(id)
        if 'device' in item['tags']:
            return self.config.devices.find_by_name(item['text'])
        else:
            return None

    def updateEditTitle(self):
        self.editFrame.configure(text="Edit %s '%s'" % (self.edit.type, self.edit.name))

    def resetEditor(self):
        f = self.editFrame
        for c in f.winfo_children():
            c.destroy()

        def update(*args):
            edit = self.edit
            try:
                obj = get_dict_value(edit.json, edit.name, {})
                for k, v in edit.items():
                    if k == 'name':
                        name = v.get()
                        if name != edit.name:
                            old = edit.json.pop(edit.name)
                            obj = edit.json[name] = old
                            edit.name = name
                            self.updateEditTitle()
                    else:
                        obj[k] = v.get()
                self.reload()
            except AttributeError as err:
                self.status.set(err)

        btn = ttk.Button(f, text="Update", command=update)
        btn.grid(row=100, column=0, columnspan=2)
        return f

    def editDevice(self, dev):
        f = self.resetEditor()
        edit = self.edit = EditState(self, 'Device', 'devices', dev.name)
        self.updateEditTitle()
        i = 0
        for k in edit.keys():
            v = edit.get_property(k)
            edit[k] = tk.StringVar(value=read_property(dev, k))
            l = tk.Label(f, text=k)
            l.grid(row=i, column=0, sticky=tk.W)
            if k == 'type':
                c = ttk.Combobox(f, values=list((storage.TYPES).keys()))
            elif 'enum' in v:
                c = ttk.Combobox(f, values=v['enum'])
            else:
                c = tk.Entry(f)
            c.configure(textvariable=edit[k])
            c.configure(state=edit.getState(k))
            c.grid(row=i, column=1, sticky=tk.EW)
            i += 1


    def editPartition(self, part):
        f = self.resetEditor()
        edit = self.edit = EditState(self, 'Partition', 'partitions', part.name)
        self.updateEditTitle()
        i = 0
        for k in edit.keys():
            v = self.edit.get_property(k)
            edit[k] = tk.StringVar(value=read_property(part, k))
            if v['type'] == 'boolean':
                c = ttk.Checkbutton(f, text = k, variable=self.edit[k])
            else:
                l = tk.Label(f, text=k)
                l.grid(row=i, column=0, sticky=tk.W)
                if k == 'device':
                    edit[k].set(part.device.name)
                    c = ttk.Combobox(f, values=[dev.name for dev in self.config.devices])
                elif k == 'type':
                    c = ttk.Combobox(f, values=list((partition.TYPES).keys()))
                elif k == 'subtype':
                    subtypes = partition.SUBTYPES.get(part.type, None)
                    if subtypes is None:
                        c = tk.Entry(f)
                    else:
                        c = ttk.Combobox(f, values=list(subtypes))
                elif 'enum' in v:
                    c = ttk.Combobox(f, values=v['enum'])
                else:
                    c = tk.Entry(f, width=64)
                c.configure(textvariable=edit[k])
            c.configure(state=edit.getState(k))
            c.grid(row=i, column=1, sticky=tk.EW)
            i += 1


def main():
    parser = argparse.ArgumentParser(description='Sming hardware profile editor')
    parser.add_argument('input', help='Name of existing hardware profile')
    args = parser.parse_args()

    root = tk.Tk()
    editor = Editor(root)
    editor.initialise()
    editor.loadConfig(args.input)
    root.mainloop()


if __name__ == '__main__':
    try:
        print("TCL version %s" % tk.TclVersion)
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
