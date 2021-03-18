import common, argparse, os, partition
from common import *
from config import *
import tkinter as tk
from tkinter import ttk

class Editor:
    def __init__(self, root):
        root.title('Sming Hardware Profile Editor')
        root.columnconfigure(0, weight=1)
        root.rowconfigure(0, weight=1)
        self.main = root

    def initialise(self):
        with open(os.environ['HWCONFIG_SCHEMA']) as f:
            self.schema = json.load(f)

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

        self.base_config = tk.StringVar(value = 'standard')
        config_list = ttk.Combobox(f, textvariable = self.base_config, values = list(get_config_list().keys()))
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

        # Status label
        status = ttk.Label(self.main, text='-')
        status.grid(row=1, column=1, rowspan=2, sticky=tk.W)
        def select(*args):
            id = tree.focus()
            item = tree.item(id)
            if 'device' in item['tags']:
                dev = self.device_from_id(id)
                s = 'Device: ' + dev.to_json()
            else:
                dev = self.device_from_id(tree.parent(id))
                addr = item['values'][0]
                part = self.config.map().find_by_address(dev, addr)
                s = 'Partition: ' + part.to_json()
            status.configure(text=s)
        tree.bind('<<TreeviewSelect>>', select)


    def loadConfig(self, config_name):
        filename = find_config(config_name)
        with open(filename) as f:
            self.json = json.loads(jsmin(f.read()))
        self.reload()


    def reload(self):
        tree = self.tree

        self.config = Config.from_json(self.json)
        config = self.config

        for c in tree.get_children():
            tree.delete(c)

        # Devices are our root nodes
        for dev in config.devices:
            tree.insert('', 'end', dev.name, text=dev.name, open=True,
                tags = ['device'],
                values=['', dev.size_str(), dev.type_str()])

        # Partitions are children
        for p in config.map():
            tree.insert(p.device.name, 'end', text=p.name if p.name != '(unused)' else '',
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
