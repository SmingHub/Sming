import common, argparse, os, partition
from common import *
from config import Config
import tkinter as tk
from tkinter import ttk

class Editor:
    def __init__(self, root):
        root.title('Sming Hardware Profile Editor')
        self.main = ttk.Frame(root)
        self.main.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        # self.createWidgets()

    def createWidgets(self):
        quitButton = tk.Button(self.main, text='Quit', command=self.main.quit)
        quitButton.pack()

    def setConfig(self, config):
        self.config = config

        # Frame to contain treeview plus vertical scrollbar
        f = ttk.Frame(self.main)
        f.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        tree = ttk.Treeview(f, columns=['address', 'size', 'type', 'subtype', 'filename'])
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        s = ttk.Scrollbar(f, orient=tk.VERTICAL, command=tree.yview)
        s.pack(side=tk.RIGHT, fill=tk.Y, expand=True)
        tree['yscrollcommand'] = s.set

        # tree.heading('#0', text='Device / Partition Name')
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
            # id = p.device.name + '/' + p.address_str()
            tree.insert(p.device.name, 'end', text=p.name if p.name != '(unused)' else '',
                tags = ['unused' if p.type == 0xff else 'normal'],
                values=[p.address_str(), p.size_str(), p.type_str(), p.subtype_str(), p.filename])

        tree.tag_configure('device', font='+1')
        # tree.tag_configure('normal', font='+1')

        def device_from_id(id):
            item = tree.item(id)
            if 'device' in item['tags']:
                return self.config.devices.find_by_name(item['text'])
            else:
                return None

        # Status label
        status = ttk.Label(self.main, text='-')
        status.pack(side=tk.BOTTOM, fill=tk.X, expand=True)
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

        self.options = {}

        def toggle_option(*args):
            critical("Do something... %s" % str(args))

        # Option checkboxes
        for k, v in self.config.option_library.items():
            self.options[k] = tk.BooleanVar(value = k in self.config.options)
            btn = tk.Checkbutton(self.main, text = k + ': ' + v['description'],
                command=toggle_option, variable=self.options[k])
            btn.pack(side=tk.BOTTOM, fill=tk.X)



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
