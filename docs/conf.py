# -- Project information -----------------------------------------------------

project = 'OCTypes'
copyright = '2025, Philip Grandinetti'
author = 'Philip Grandinetti'
release = '0.1.0'

primary_domain = 'c'

# -- General configuration ---------------------------------------------------

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'breathe',
]

# Remove broken intersphinx mapping (cppreference doesn't provide objects.inv)
intersphinx_mapping = {
    'python': ('https://docs.python.org/3', None),
    # 'c': ('https://en.cppreference.com/w/c', None),
}

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

suppress_warnings = [
    'duplicate_declaration.c',
]

# -- Breathe Configuration ---------------------------------------------------

breathe_projects = {
    "OCTypes": "doxygen/xml"
}
breathe_default_project = "OCTypes"
breathe_verbose = False
breathe_default_members = ('members', 'undoc-members')  # Ensure this line is active
breathe_default_domain = 'c'
breathe_domain_by_extension = {
    'h': 'c',
    'c': 'c',
}

# -- Options for HTML output -------------------------------------------------

html_theme = 'sphinx_rtd_theme'
# html_static_path = ['_static'] # Commented out to resolve "does not exist" error
