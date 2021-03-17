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
        self.createWidgets()

    def createWidgets(self):
        quitButton = tk.Button(self.main, text='Quit', command=self.main.quit)
        quitButton.pack()

    def setConfig(self, config):
        self.config = config

        f = ttk.Frame(self.main)
        f.pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        tree = ttk.Treeview(f, columns=('address', 'size', 'type', 'subtype', 'filename'))
        tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        s = ttk.Scrollbar(f, orient=tk.VERTICAL, command=tree.yview)
        s.pack(side=tk.RIGHT, fill=tk.Y, expand=True)
        tree['yscrollcommand'] = s.set

        tree.heading('#0', text='Device / Partition Name')
        tree.heading('address', text='Address')
        tree.heading('size', text='Size')
        tree.heading('type', text='Type')
        tree.heading('subtype', text='Sub-Type')
        tree.heading('filename', text='Image filename')

        # tree.insert('', 'end', 'widgets', text='Widget Tour')

        for dev in config.devices:
            tree.insert('', 'end', dev.name, text=dev.name, open=True)

        for p in config.map():
            # id = p.device.name + '/' + p.address_str()
            tree.insert(p.device.name, 'end', text=p.name,
                values=(p.address_str(), p.size_str(), p.type_str(), p.subtype_str(), p.filename))

        # Option checkboxes

        for k, v in self.config.option_library.items():
            btn = tk.Checkbutton(self.main, text = k + ': ' + v['description'])
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
