#!/usr/bin/env python
# encoding: utf-8
"""
setup.py -- setup file for the pyics module

Created by Tim van Werkhoven (t.i.m.vanwerkhoven@xs4all.nl).
Copyright (c) 2009 Tim van Werkhoven. All rights reserved.
"""
import sys

try:
	import numpy
	from numpy.distutils.core import setup, Extension
except:
	print "Could not load NumPy (numpy.distutils.core), required by this package. Aborting"
	sys.exit(1)

module_pyics = Extension('_pyics',
                    define_macros = [('MAJOR_VERSION', '0'),
                                     ('MINOR_VERSION', '1')],
                    include_dirs = [numpy.get_include()],
                   	libraries = ["ics"],
#                   library_dirs = [''],
										extra_compile_args=["-O3", "-ffast-math"],
										extra_link_args=None,
                    sources = ['src/_pyics.c'])

setup (name = 'pyics',
	version = '0.1.0a',
	description = 'Python library to read ICS files',
	author = 'Tim van Werkhoven',
	author_email = 't.i.m.vanwerkhoven@xs4all.nl',
	url = '',
	license = "GPL",
# This is for the python wrapper module:
  package_dir = {'pyics' : 'pyics'},
  packages = ['pyics'],
  ext_package = 'pyics',
# This is for the C module
	ext_modules = [module_pyics])
