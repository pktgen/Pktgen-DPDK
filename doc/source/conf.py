# -*- coding: utf-8 -*-

from sphinx.highlighting import PygmentsBridge
from pygments.formatters.latex import LatexFormatter


project = 'Pktgen'
copyright = '2010-2024, Keith Wiles'

# Version of Pktgen-DPDK
version = '23.06.1'
release = version

extensions = []

source_suffix = {
    '.rst': 'restructuredtext',
}

# Optional Markdown support for developer-friendly docs.
# If myst_parser is installed, Sphinx can render .md sources referenced by
# toctrees (e.g. docs/QUICKSTART.md and lib/cli/DESIGN.md).
try:
    import myst_parser  # noqa: F401
except Exception:  # pragma: no cover
    pass
else:
    extensions.append('myst_parser')
    source_suffix['.md'] = 'markdown'

initial_doc = 'index'
pygments_style = 'sphinx'
html_theme = 'default'
html_add_permalinks = ''
htmlhelp_basename = 'Pktgendoc'

latex_documents = [
    ('index',
     'pktgen.tex',
     'Pktgen-DPDK Documentation',
     'Keith Wiles', 'manual'),
]

latex_preamble = """
\\usepackage{upquote}
\\usepackage[utf8]{inputenc}
\\usepackage{DejaVuSansMono}
\\usepackage[T1]{fontenc}
\\usepackage{helvet}
\\renewcommand{\\familydefault}{\\sfdefault}

\\RecustomVerbatimEnvironment{Verbatim}{Verbatim}{xleftmargin=5mm}
"""

latex_elements = {
    'papersize': 'a4paper',
    'pointsize': '11pt',
    'preamble': latex_preamble,
}


man_pages = [
    ('index',
     'pktgen',
     'Pktgen Documentation',
     ['Keith Wiles'],
     1)
]

texinfo_documents = [
    ('index', 'Pktgen',
     'Pktgen Documentation',
     'Keith Wiles',
     'Pktgen',
     'One line description of project.',
     'Miscellaneous'),
]


class CustomLatexFormatter(LatexFormatter):
    def __init__(self, **options):
        super(CustomLatexFormatter, self).__init__(**options)
        self.verboptions = r"formatcom=\footnotesize"

PygmentsBridge.latex_formatter = CustomLatexFormatter
