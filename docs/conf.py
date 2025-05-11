# docs/conf.py

# -- Project information -----------------------------------------------------

project = 'OCTypes'
copyright = '2025, Philip Grandinetti'
author = 'Philip Grandinetti'
release = '0.1.0'

# Use the C domain by default
primary_domain = 'c'


# -- General configuration ---------------------------------------------------

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'breathe',
]

# Only Python intersphinx for now
intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
}

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# Silence exactly that “Duplicate C declaration” warning (substring match)
suppress_warnings = [
    'Duplicate C declaration',
]


# -- Breathe configuration ---------------------------------------------------

breathe_projects = {
    "OCTypes": "doxygen/xml"
}
breathe_default_project = "OCTypes"
breathe_verbose = False
breathe_default_members = ('members', 'undoc-members')
breathe_default_domain = 'c'
breathe_domain_by_extension = {
    'h': 'c',
    'c': 'c',
}


# -- HTML output --------------------------------------------------------------

html_theme = 'sphinx_rtd_theme'
# html_static_path = ['_static']  # uncomment if you actually have a _static dir
