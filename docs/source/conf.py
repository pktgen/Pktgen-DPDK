# -*- coding: utf-8 -*-

from sphinx.highlighting import PygmentsBridge
from pygments.formatters.latex import LatexFormatter


project = 'Pktgen'
copyright = '2015, Keith Wiles'

version = '2.7.7'
release = version

source_suffix = '.rst'
master_doc = 'index'
pygments_style = 'sphinx'
html_theme = 'default'
html_add_permalinks = ''
htmlhelp_basename = 'Pktgendoc'

latex_documents = [
    ('index',
     'pktgen.tex',
     'Pktgen Documentation',
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
