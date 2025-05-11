# Configuration file for the Sphinx documentation builder.
# https://www.sphinx-doc.org/en/master/usage/configuration.html

import os
import sys
import warnings

# -- Path setup --------------------------------------------------------------
# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here.
sys.path.insert(0, os.path.abspath('..'))

# -- Project information -----------------------------------------------------
project = 'octypes'
author = 'pjgrandinetti'
release = 'latest'

# -- General configuration ---------------------------------------------------
# List of Sphinx extension module names
extensions = [
    'breathe',
]

# Templates path
templates_path = ['_templates']

# Patterns to exclude
exclude_patterns = []

# -- Breathe configuration ---------------------------------------------------
# Point Breathe at the Doxygen XML output
breathe_projects = {
    'octypes': '../doxygen/xml'
}
breathe_default_project = 'octypes'

# -- Warning suppression -----------------------------------------------------
# Silently ignore duplicate C declarations (they come from multiple modules)
warnings.filterwarnings(
    'ignore',
    message=r'.*Duplicate C declaration.*',
    category=Warning
)

# -- Options for HTML output -------------------------------------------------
html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
