# Configuration file for the Sphinx documentation builder.

import os
import sys

# Add project root to sys.path for autodoc or other extensions
sys.path.insert(0, os.path.abspath('..'))

# -- Project information -----------------------------------------------------
project = 'OCTypes'
author = 'pjgrandinetti'
release = 'latest'

# -- General configuration ---------------------------------------------------
extensions = [
    'breathe',
    'sphinx_rtd_theme',
]

# Breathe configuration for Doxygen XML output
breathe_projects = {
    'OCTypes': os.path.abspath('doxygen/xml')
}
breathe_default_project = 'OCTypes'

# Suppress specific warnings so builds don’t break on duplicate C declarations
suppress_warnings = [
    'c.duplicate_declaration',  # ignore duplicate C declarations from Doxygen
]

# Templates and static paths
templates_path = ['_templates']
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']

