# -*- coding: utf-8 -*-
#

extensions = [
    'breathe',
    'exhale',
    'sphinx_rtd_theme',
]

breathe_projects = {"libxxx": "../doxygen/xml"}
breathe_default_project = "libxxx"

exhale_args = {
    "containmentFolder": "./api",
    "rootFileName": "index.rst",
    "rootFileTitle": "libxxx API",
    "doxygenStripFromPath": ".",
    "createTreeView": True,
    "exhaleExecutesDoxygen": True,
    "exhaleUseDoxyfile": True,
}
templates_path = ['_templates']
source_suffix = '.rst'
source_encoding = 'utf-8-sig'
master_doc = 'index'
project = u'libxxx'
copyright = u'2023, Ledger SA'
author = u'Ledger'
version = u'1.0.0'
release = u'1.0.0'

language = None
today_fmt = '%B %d, %Y'
exclude_patterns = []
pygments_style = 'sphinx'
todo_include_todos = True

html_theme = 'sphinx_rtd_theme'
html_logo = ''
html_static_path = ['_static']
html_use_smartypants = True
html_show_sourcelink = False
html_show_sphinx = False
htmlhelp_basename = 'libxxxdoc'

man_pages = [
    (master_doc, 'xxx', u'Libxxx Documentation',
     [author], 1)
]


tags.add('v1.0.0')
