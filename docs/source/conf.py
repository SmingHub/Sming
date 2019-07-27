# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# http://www.sphinx-doc.org/en/master/config

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import re
import os
import sys
import subprocess
from docutils import nodes, utils
from sphinx import roles, addnodes
from sphinx.util.nodes import set_role_source_info, split_explicit_title
# So our custom extensions can be found
sys.path.insert(0, os.path.abspath('.'))


# -- Project information -----------------------------------------------------

project = 'Sming'
copyright = 'Sming Developer Team'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# Tip from https://protips.readthedocs.io/git-tag-version.html to get version from tag
release = re.sub('^v', '', os.popen('git describe').read().strip())
# The short X.Y version.
version = release

# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
    'm2r',
    'breathe',
    'link-roles',
]

source_suffix = {
    '.rst': 'restructuredtext',
    '.txt': 'markdown',
    '.md': 'markdown',
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = [
]

#
master_doc = 'index'

# -- Options for breathe --

breathe_projects =  {
    "api": "../api/xml/"
}

breathe_default_project = "api"


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
html_theme = 'sphinx_rtd_theme'

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
html_logo = "sming-logo.png"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']

html_extra_path = [
    '../api/html'
]


##
# -- Use sphinx_rtd_theme for local builds --------------------------------
# ref. https://github.com/snide/sphinx_rtd_theme#using-this-theme-locally-then-building-on-read-the-docs
#
# on_rtd is whether we are on readthedocs.org
env_readthedocs = os.environ.get('READTHEDOCS', None)
print(env_readthedocs)

if env_readthedocs:
    # Start the build from a clean slate
    subprocess.call('make -C ../../Sming dist-clean SMING_ARCH=Host', shell=True)
else:
    # only import and set the theme if we're building docs locally
    import sphinx_rtd_theme
    html_theme = 'sphinx_rtd_theme'
    html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]

subprocess.call('make -C ../../Sming submodules SMING_ARCH=Host', shell=True)
subprocess.call('make -C .. setup api API_VERSION="' + version + '"', shell=True)

def setup(app):
    app.add_stylesheet('custom.css')
