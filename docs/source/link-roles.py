#
# Custom roles for simplified linking to parts of Sming
#
# autolink from https://github.com/espressif/esp-idf/blob/master/docs/link-roles.py
#
# doclink mikee47 <mike@sillyhouse.net>
#

import re
import os
import sys
from docutils import nodes, utils
from sphinx import roles, addnodes
from sphinx.util.nodes import set_role_source_info, split_explicit_title
from sphinx.util import logging

logger = logging.getLogger(__name__)

github_url = 'https://github.com/SmingHub/Sming'

def run_cmd_get_output(cmd):
    return os.popen(cmd).read().strip()

def get_github_rev():
    path = run_cmd_get_output('git rev-parse --short HEAD')
    tag = run_cmd_get_output('git describe --exact-match')
    print('Git commit ID: ', path)
    if len(tag):
        print('Git tag: ', tag)
        path = tag
    return path


def setup(app):
    app.add_role('source', SourceRole())
    app.add_role('issue', autolink('Issue #{0} <' + github_url + '/issues/{0}>'))
    app.add_role('pull-request', autolink('Pull Request #{0} <' + github_url + '/pull/{0}>'))

    app.add_role('sample', SampleRole)
    app.add_role('component', doclink('/_inc/Sming/Components/{}/index'))
    app.add_role('component-esp8266', doclink('/_inc/Sming/Arch/Esp8266/Components/{}/index'))
    app.add_role('component-esp32', doclink('/_inc/Sming/Arch/Esp32/Components/{}/index'))
    app.add_role('component-host', doclink('/_inc/Sming/Arch/Host/Components/{}/index'))
    app.add_role('component-rp2040', doclink('/_inc/Sming/Arch/Rp2040/Components/{}/index'))
    app.add_role('library', doclink('/_inc/Sming/Libraries/{}/index'))

def autolink(pattern):
    """Insert a link to a file or issue in the repo.
    Both pattern and user text may use optional format, e.g. `title <link {}>`
    """

    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        text_has_explicit_title, link_text, link = split_explicit_title(text)
        pattern_has_explicit_title, pattern_text, pattern_link = split_explicit_title(pattern)
        url = pattern_link.format(link)
        if not text_has_explicit_title and pattern_has_explicit_title:
            link_text = pattern_text.format(link)
        node = nodes.reference(rawtext, link_text, refuri=url, **options)
        return [node], []
    return role


def doclink(pattern):
    """Default title is extracted from target document."""

    def DocumentRole(typ, rawtext, text, lineno, inliner, options={}, content=[]):
        # type: (unicode, unicode, unicode, int, Inliner, Dict, List[unicode]) -> Tuple[List[nodes.Node], List[nodes.Node]]  # NOQA
        env = inliner.document.settings.env
        # split title and target in role content
        has_explicit_title, title, target = split_explicit_title(text)
        title = utils.unescape(title)
        target = pattern.format(utils.unescape(target))
        # create the reference node
        refnode = addnodes.pending_xref(rawtext, reftype='doc', refdomain='std', refexplicit=has_explicit_title)
        # we may need the line number for warnings
        set_role_source_info(inliner, lineno, refnode)  # type: ignore
        # now that the target and title are finally determined, set them
        refnode['reftarget'] = target
        refnode += nodes.inline(rawtext, title, classes=['xref', 'doc'])
        # we also need the source document
        refnode['refdoc'] = env.docname
        refnode['refwarn'] = True
        return [refnode], []

    return DocumentRole


def SampleRole(typ, rawtext, text, lineno, inliner, options={}, content=[]):
    """Sample may be in current component/library or main sample repository."""

    # split title and target in role content
    has_explicit_title, title, target = split_explicit_title(text)
    title = utils.unescape(title)
    target = utils.unescape(target)

    env = inliner.document.settings.env
    pageUrls = env.config.html_context['page_urls']
    docname = env.docname

    # Target can be "{sample name}" or "{component name}/{sample name}"
    if target.startswith('/'):
        expr = f"_inc/samples{target}/index.rst"
    else:
        segs = target.split('/')
        if len(segs) == 2:
            expr = f"/{segs[0]}/samples/{segs[1]}/index.rst"
        else:
            expr = f"/samples/{target}/index.rst"

    # Search for all sample matches
    pageList = list(filter(lambda page: page.endswith(expr), pageUrls))

    logger.verbose(f">> :sample:`{target}` found {pageList}")

    sampleTarget = None
    if len(pageList) == 1:
        # One match, use it
        sampleTarget = pageList[0]
    elif len(pageList) > 1:
        # Multiple matches: we have some decisions to make
        # If document page is in a Component then check that first
        cmp = getComponentPath(docname)
        if cmp is not None:
            sampleTarget = next(filter(lambda page: page.startswith(cmp), pageList), None)
        # Not found? Then check main samples directory
        if sampleTarget is None:
            sampleTarget = next(filter(lambda page: page.startswith('_inc/samples'), pageList), None)
        # Still not found? Default to first match
        if sampleTarget is None:
            sampleTarget = pageList[0]

    # No match found, leave target unchanged
    if sampleTarget is None:
        sampleTarget = target
    else:
        # Absolute path required, without extension
        sampleTarget = "/" + os.path.splitext(sampleTarget)[0]

    logger.verbose(f">> sample '{target}' -> '{sampleTarget}'")

    # create the reference node
    refnode = addnodes.pending_xref(rawtext, reftype='doc', refdomain='std', refexplicit=has_explicit_title)
    # we may need the line number for warnings
    set_role_source_info(inliner, lineno, refnode)  # type: ignore
    # now that the target and title are finally determined, set them
    refnode['reftarget'] = sampleTarget
    refnode += nodes.inline(rawtext, title, classes=['xref', 'doc'])
    # we also need the source document
    refnode['refdoc'] = docname
    refnode['refwarn'] = True
    return [refnode], []


def getComponentPath(path):
    """Obtain name of Component or library from document path.
    Return None if document is not in a Component or library.
    """

    a, b, c = path.partition("/Components/")
    if b == "":
        a, b, c = path.partition("/Libraries/")
    if b == "":
        return None

    return f"{a}{b}{c.split('/')[0]}"


def SourceRole():
    """Create hyperlink to source code, which may be in a submodule."""

    def role(name, rawtext, text, lineno, inliner, options={}, content=[]):
        _, link_text, link = split_explicit_title(text)

        env = inliner.document.settings.env
        rootUrls = env.config.html_context['root_urls']
        rootPaths = env.config.html_context['root_paths']

        def getRoot(srcpath):
            path = next(filter(link.startswith, rootPaths), '')
            return rootUrls[path], link[len(path):]

        if link.startswith('/'):
            # Absolute links are relative to Sming repo
            linkUrl = rootUrls[''] + link
        else:
            # Resolve link paths within Components
            path = getComponentPath(env.docname)
            if path is not None:
                path = path[5:] # skip '_inc/'
                if not link.startswith(path):
                    # print(f">> {link} -> {path}/{link}")
                    link = f"{path}/{link}"
            linkUrl, linkPath = getRoot(link)
            linkUrl = f"{linkUrl}/{linkPath}"

        logger.verbose(f">> source '{link}' -> '{linkUrl}'")
        node = nodes.reference(rawtext, link_text, refuri=linkUrl, **options)
        return [node], []


    return role
