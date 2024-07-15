Continuous Integration testing
==============================

Github actions
--------------

See ``.github/workflows``.

Cache clean
      Dispatch workflow as convenient way to clean action cache. Cleaning levels are:

      - pull-requests
            Any items created by pull requests, excludes anything in develop branch.
            Normally pull requests will make use of caches present in the develop branch, but if there isn't one it will create its own. This becomes redundant when merged to develop.

            Such caches should be expired automatically, but this option can be used to remove them.

      - ccache
            All ccache items. Use if pull requests are taking too long to build.

      - idf-tools
            All IDF tool. Use before merging to develop if IDF toolchains have been updated.

      - ccache+idf
            All ccache and IDF tool caches.

      Note that cleaning can always be done locally using ``gh``::

            gh cache list # For fork
            gh cache list -R SmingHub/Sming
            gh cache delete --all -R SmingHub/Sming

      etc.


CodeQL
      Performs code quality analysis when develop branch is updated.

Continuous Integration (CI)
      Tests for all architectures except esp32. Run for every pull request and merge to develop.

Continuous Integration (CI) for Esp32
      Tests for esp32 architecture. Requires a separate workflow as matrix becomes too complex otherwise.

Continuous Integration (CI) for Library
      Used indirectly by library workflows for testing. See workflows/library.

Coverity Scan
      Code quality analyser. Does instrumented build then uploads database to coverity servers for analysis.

Release
      Run manually during release phase only.

Spelling Check
      Run for all pull requests and merge to develop.


Esp32 IDF and tools cleaning
----------------------------

Because the IDF and associated tools are large and relatively time-consuming to install, these are cached.
There's so much bloat that it doesn't take much to fill the 10GB github cache allocation.

So after installing the tools - before it gets cached - the `clean-tools.py` script gets run.
This tool contains a list of filters (regular expressions) which match various paths.
Candidates for removal were identified by inspection using the Gnome disk usage analyzer.
Some other stuff (like test code and examples) are fairly safe candidates to remove as well.

To evaluate how much would be removed run this command (it's safe)::

      python clean-tools.py scan

To perform 'dry-run' of a clean operation, without actually deleteing anything::

      python clean-tools.py clean

To actually delete stuff requires a confirmation flag::

      python clean-tools.py clean --delete

Note that some unused submodules are cleaned, but by default the IDF pulls them back in again!
To prevent this behaviour, set `IDF_SKIP_CHECK_SUBMODULES=1`.


CI logs
-------

Analysing CI logs is important for several reasons:

Identifying sudden jumps in memory usage between builds
    This can indicate a potential problem with SDK updates or the build system

Checking warnings
    - Compiler warnings are a very important indicator of potential problems in code.
    - CI runs are built in STRICT mode so despite best efforts there can be many remaining messages.
    - The same warning in a particular source file can occur multiple times in a run and across
      multiple runs

Logs can be downloaded via the web browser in 'raw' format, but getting these takes manual
effort and has to be repeated for each build: currently there are 42 builds across two runs.

Fortunately, github actions provides a CLI application to do this.
The log files related to a run and therefore only two are required (the main build, and esp32).


Setup
~~~~~

The github CLI client must be installed and authenticated with the Sming repo (or fork).

See https://docs.github.com/en/github-cli/github-cli/quickstart.


Usage
~~~~~

Fetch and scan the most recent build::

      python3 scanlog.py last-build.txt --fetch

This will download all CI runs from the most recent workflow into ``last-build.txt``, then parse it.
If the file already exists, downloading will be skipped.
Output is to console and can be redirected to a file if required.

To compare with another previously fetched set of logs::

      python3 scanlog.py last-build.txt -c previous-build.txt

To fetch a specific build::

      python3 scanlog.py custom-fixes.txt --fetch --branch feature/custom-fixes

To explicitly specify the repository to fetch from::

      python3 scanlog.py custom-fixes.txt --fetch --branch feature/custom-fixes --repo SmingHub/Sming

To list all source locations with warnings::

      python3 scanlog.py last-build.txt -w -m

Note: The 'm' flag merges warnings from all jobs. Omitting this shows warnings for each job separately.

To filter out warnings::

      python3 scanlog.py last-build.txt -w -m --exclude warn-exclude.lst

The named exclusion file contains a list of regular expressions to match against.


vscode
~~~~~

The warnings output using the scanlog tool can be used as hyperlinks in vscode:

- Select a project, e.g. ``tests/HostTests`` and run ``make ide-vscode``
- Open the resulting workspace in vscode
- Add the ``sming`` folder to the project
- Open an integrated terminal and dump the warnings as shown above.
  Or, redirect them into a file and ``cat`` it.

The file locations act as links to the source.
Note that this isn't perfect. For example, esp-idf paths are not resolved to the specific version in use.
Listing warnings for each job can be helpful as it shows which IDF version was used.
