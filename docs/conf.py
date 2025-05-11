# Configuration file for the Sphinx documentation builder.
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'OCTypes'
copyright = '2025, Philip Grandinetti'
author = 'Your Name'
release = '0.1.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.intersphinx',
    'breathe'  # Added breathe here
]

# Breathe Configuration
breathe_projects = {
    "OCTypes": "doxygen/xml/" # Path to Doxygen XML output, relative to the conf.py file
}
breathe_default_project = "OCTypes"

templates_path = ['_templates']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme' # Set the theme to Read the Docs theme
html_static_path = ['_static']