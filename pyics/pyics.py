#!/usr/bin/env python
# encoding: utf-8
"""
pyics.py

Created by Tim van Werkhoven (t.i.m.vanwerkhoven@xs4all.nl).
Copyright (c) 2009 Tim van Werkhoven. All rights reserved.
"""

## Import libraries

import sys
import os
import unittest
import _pyics

## Functions for loading in ANA files

def read(filename, debug=0):
	"""
	Load an ICS file and return the data and header in a dict.
	
	data = pyics.load(filename)
	"""
	if not os.path.isfile(filename):
		raise IOError("File does not exist!")
	
	data = _pyics.read(filename, debug)
	return data


def getdata(filename, debug=0):
	"""
	Load an ICS file and only return the data as a Numpy array.
	
	data = pyics.getdata(filename)
	"""
	return (read(filename, debug))['data']

	
def getheader(filename, debug=0):
	"""
	Load an ICS file and only return the header.
	
	header = pyics.getheader(filename)
	"""
	return (read(filename, debug))['header']


## Functions for storing ICS files
def write(filename, data, compress=0, comments=False, debug=0):
	"""
	Save a 2D numpy array as an ICS file and return the bytes written, or NULL 
	on failure.
	
	written = pyics.write(filename, data, compress=0, comments=False)
	"""
	if (comments):
		return _pyics.write(filename, data, compress, comments, debug)
	else:
		return _pyics.write(filename, data, compress, '', debug)


## Selftesting using unittest starts below this line
# class pyicsTests(unittest.TestCase):
# 	def setUp(self):
# 		# Create a test image, store it, reread it and compare
# 		import numpy as N
# 		self.numpy = N
# 		self.img_size = (456, 345)
# 		self.img_src = N.arange(N.product(self.img_size))
# 		self.img_src.shape = self.img_size
# 		self.img_i8 = self.img_src*2**8/self.img_src.max()
# 		self.img_i8 = self.img_i8.astype(N.int8)
# 		self.img_i16 = self.img_src*2**16/self.img_src.max()
# 		self.img_i16 = self.img_i16.astype(N.int16)
# 		self.img_f32 = self.img_src*1.0/self.img_src.max()
# 		self.img_f32 = self.img_f32.astype(N.float32)
# 	
# 	def runTests(self):
# 		unittest.TextTestRunner(verbosity=2).run(self.suite())
# 	
# 	def suite(self):
# 		suite = unittest.TestLoader().loadTestsFromTestCase(pyicsTests)
# 		return suite
# 	
# 	def testi8c(self):
# 		# Test int 8 compressed functions
# 		write('/tmp/pyics-testi8c', self.img_i8, 1, 'testcase', 1)
# 		self.img_i8c_rec = read('/tmp/pyics-testi8c', 1)
# 		self.assert_(self.numpy.sum(self.img_i8c_rec['data'] - self.img_i8) == 0,
# 		 	msg="Storing 8 bits integer data with compression failed (diff: %d)" % (self.numpy.sum(self.img_i8c_rec['data'] - self.img_i8)))
# 	
# 	def testi8u(self):
# 		# Test int 8 uncompressed functions
# 		write('/tmp/pyics-testi8u', self.img_i8, 0, 'testcase', 1)
# 		self.img_i8u_rec = read('/tmp/pyics-testi8u', 1)
# 		self.assert_(self.numpy.sum(self.img_i8u_rec['data'] - self.img_i8) == 0,
# 			msg="Storing 8 bits integer data without compression failed (diff: %d)" % (self.numpy.sum(self.img_i8u_rec['data'] - self.img_i8)))
# 	
# 	def testi16c(self):
# 		# Test int 16 compressed functions
# 		write('/tmp/pyics-testi16c', self.img_i16, 1, 'testcase', 1)
# 		self.img_i16c_rec = read('/tmp/pyics-testi16c', 1)
# 		self.assert_(self.numpy.allclose(self.img_i16c_rec['data'], self.img_i16),
# 			msg="Storing 16 bits integer data with compression failed (diff: %d)" % (self.numpy.sum(self.img_i16c_rec['data'] - self.img_i16)))
# 	
# 	def testi16u(self):
# 		# Test int 16 uncompressed functions
# 		write('/tmp/pyics-testi16u', self.img_i16, 0, 'testcase', 1)
# 		self.img_i16u_rec = read('/tmp/pyics-testi16u', 1)
# 		self.assert_(self.numpy.allclose(self.img_i16u_rec['data'], self.img_i16),
# 			msg="Storing 16 bits integer data without compression failed (diff: %d)" % (self.numpy.sum(self.img_i16u_rec['data'] - self.img_i16)))
# 	
# 	def testf32u(self):
# 		# Test float 32 uncompressed functions
# 		write('/tmp/pyics-testf32', self.img_f32, 0, 'testcase', 1)
# 		self.img_f32_rec = read('/tmp/pyics-testf32', 1)
# 		self.assert_(self.numpy.allclose(self.img_f32_rec['data'], self.img_f32),
# 			msg="Storing 32 bits float data without compression failed (diff: %g)" % (1.0*self.numpy.sum(self.img_f32_rec['data'] - self.img_f32)))
# 	
# 	def testf32c(self):
# 		# Test if float 32 compressed fails
# 		self.assertRaises(RuntimeError, write, '/tmp/pyics-testf32', self.img_f32, 1, 'testcase', 1)
# 		
# 
# if __name__ == "__main__":	
# 	suite = unittest.TestLoader().loadTestsFromTestCase(pyicsTests)
# 	unittest.TextTestRunner(verbosity=2).run(suite)
	

