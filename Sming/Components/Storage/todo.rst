TODO
====

Access filesystem builders via plugins
   e.g. add 'build' button to partition edit dialog
   Instead of building via the makefile (from where we've been called),
   this should be done via plugins.
   Plugins are defined in schema (in the 'spiffs' and 'IFS' modules).

Add flash support to editor
   e.g. add 'read', 'write', 'erase' buttons to partition edit dialog.
   Can also add these to device which will do the same for all contained partitions.
   For example, select one or more partitions and click 'read', 'write', etc.

Enable multi-select?
   Allow selecting multiple partitions for read/write.

Support reading/writing external storage devices
   Add fields to .hw which define how this is done.
   This could be done with build targets or shell commands.

Filesystem introspection
   For SPIFFS, partitions are typically shown as FULL rather than actual filesystem usage.
   Plugins can be used to allow more useful information to be shown,
   and allow direct browsing (and editing?) of filesystem content.
