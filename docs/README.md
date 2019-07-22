# Sming Documentation

This directory contains the build system for Sming's documentation.
It is intended to be read online via [Read The Docs](https://sming.readthedocs.io).

The `source` directory contains some top-level files however most of the
information is obtained from the various README files associated with the source code.

## Setup

### make

Should already be available as part of Sming build system

### doxygen & graphviz

For Linux:

	sudo apt install doxygen graphviz-dev

For Windows, see:

* *Doxygen* [www.doxygen.nl](http://www.doxygen.nl/)
* *Graphviz* [graphviz.org](https://graphviz.org/)

GraphViz is used by Doxygen (and Sphinx) to render graphs from .dot files.

### Python 3 + Sphinx

For Linux:

	sudo apt install python3-sphinx python3-pip python3-setuptools

For Windows, see [Sphinx Installation Guide](https://www.sphinx-doc.org/en/master/usage/installation.html).

### Requirements

Install other required packages:

	pip3 install -r requirements.txt


## Building

Build the documentation like this:

	make html

This will:

* Pull in and patch every submodule
* Generate doxygen API information (in XML format)
* Build sphinx documents in HTML format

If you make changes to any source documentation files these will be
picked up automatically. However, if you make any changes to source
file documentation then you'll need to re-build the doxygen information
like this:

	make api -B
	make html
