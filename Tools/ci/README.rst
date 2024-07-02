CI logs
=======

Overview
--------

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
-----

The github CLI client must be installed and authenticated with the Sming repo (or fork).

See https://docs.github.com/en/github-cli/github-cli/quickstart.


Usage
-----

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

      python3 scanlog.py last-build.txt -w

To filter out warnings::

      python3 scanlog.py last-build.txt -w --exclude warn-exclude.lst

The named exclusion file contains a list of regular expressions to match against.
