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

    def setConfig(self, config):
        self.config = config

        with open(os.environ['HWCONFIG_SCHEMA']) as f:
            self.schema = json.load(f)

        tree = ttk.Treeview(self.main, columns=['address', 'size', 'type', 'subtype', 'filename'])
        tree.grid(row=0, column=0, columnspan=2, sticky=tk.NSEW)

        s = ttk.Scrollbar(self.main, orient=tk.VERTICAL, command=tree.yview)
        s.grid(row=0, column=2, sticky=tk.NS)
        tree['yscrollcommand'] = s.set

        tree.heading('address', text='Address')
        tree.heading('size', text='Size')
        tree.heading('type', text='Type')
        tree.heading('subtype', text='Sub-Type')
        tree.heading('filename', text='Image filename')

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

        tree.tag_configure('device', font='+1')
        # tree.tag_configure('normal', font='+1')

        # Base configurations
        f = ttk.LabelFrame(self.main, text = 'Base Configuration')
        f.grid(row=1, column=0, sticky=tk.W)

        self.base_config = tk.StringVar(value = config.base_config)
        config_list = ttk.Combobox(f, textvariable = self.base_config, values = list(get_config_list().keys()))
        config_list.grid()

        # Option checkboxes

        f = ttk.LabelFrame(self.main, text = 'Options')
        f.grid(row=2, column=0, sticky=tk.W)

        def toggle_option(*args):
            critical("Do something... %s" % str(args))

        self.options = {}
        for k, v in load_option_library().items():
            self.options[k] = tk.BooleanVar(value = k in self.config.options)
            btn = tk.Checkbutton(f, text = k + ': ' + v['description'],
                command=toggle_option, variable=self.options[k])
            btn.grid(sticky=tk.W)


        def device_from_id(id):
            item = tree.item(id)
            if 'device' in item['tags']:
                return self.config.devices.find_by_name(item['text'])
            else:
                return None

        # Status label
        status = ttk.Label(self.main, text='-')
        status.grid(row=1, column=1, rowspan=2, sticky=tk.W)
        def select(*args):
            id = tree.focus()
            item = tree.item(id)
            if 'device' in item['tags']:
                dev = device_from_id(id)
                s = 'Device: ' + dev.to_json()
            else:
                dev = device_from_id(tree.parent(id))
                addr = item['values'][0]
                part = self.config.map().find_by_address(dev, addr)
                s = 'Partition: ' + part.to_json()
            status.configure(text=s)
        tree.bind('<<TreeviewSelect>>', select)


def main():
    parser = argparse.ArgumentParser(description='Sming hardware profile editor')
    parser.add_argument('input', help='Name of existing hardware profile')
    args = parser.parse_args()
    config = Config.from_name(args.input)

    root = tk.Tk()
    Editor(root).setConfig(config)
    root.mainloop()


if __name__ == '__main__':
    try:
        print("TCL version %s" % tk.TclVersion)
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
