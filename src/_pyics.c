/* 
Created by Tim van Werkhoven (t.i.m.vanwerkhoven@xs4all.nl).
Copyright (c) 2009 Tim van Werkhoven. All rights reserved.
*/

/* 
@brief A C extension for Python to read ICS files
@author Tim van Werkhoven
*/

// Headers
#include <Python.h>                   // For python extension
#include <numpy/arrayobject.h>        // For numpy
#include <libics.h>                   // For ICS I/O

// Prototypes
static PyObject * pyics_read(PyObject *self, PyObject *args);
static PyObject * pyics_write(PyObject *self, PyObject *args);

// Methods table for this module
static PyMethodDef PyanaMethods[] = {
  {"read",  pyics_read, METH_VARARGS, "Load an ICS file."},
  {"write",  pyics_write, METH_VARARGS, "Save data to an ICS file."},
  {NULL, NULL, 0, NULL}                // Sentinel
};


// Init module methods
PyMODINIT_FUNC init_pyics(void) {
  (void) Py_InitModule("_pyics", PyanaMethods);
	// Init numpy usage
	import_array();
}

/* 
 @brief load an ICS file data and header
 @param [in] filename
 @param [in] (optional) debug flag
 @return data, NULL on failure
 */
static PyObject *pyics_read(PyObject *self, PyObject *args) {
	// Function arguments
	char *filename;
  int debug=0, d;
  
  ICS *ip;
  Ics_DataType dt;
  int ndims;
  size_t dims[ICS_MAXDIM];
  size_t bufsize;
  void *buf;
  Ics_Error retval;
  
	PyArrayObject *outdata;							// Final output ndarray
	
	// Parse arguments
	if (!PyArg_ParseTuple(args, "s|i", &filename, &debug))
		return NULL;
	
	// Read file
	if (debug) printf("pyics_read: Reading in file '%s'...\n", filename);
	
  retval = IcsOpen (&ip, filename, "r");
  if (retval != IcsErr_Ok) {
    PyErr_SetString(PyExc_ValueError, "In pyics_read: could not open file.");
    return NULL;
  }
  
  IcsGetLayout (ip, &dt, &ndims, dims);
  bufsize = IcsGetDataSize (ip);
  buf = malloc (bufsize);
  
  retval = IcsGetData (ip, buf, bufsize);
  if (retval != IcsErr_Ok) {
    PyErr_SetString(PyExc_ValueError, "In pyics_read: could not read data.");
		free(buf);
    return NULL;
  }
  
  retval = IcsClose (ip);
  if (retval != IcsErr_Ok)  {
    PyErr_SetString(PyExc_ValueError, "In pyics_read: could not close file.");
		free(buf);
    return NULL;
  }    
	
	npy_intp npy_dims[ndims];           // Dimensions array 
	int npy_type;                       // Numpy datatype
	
	if (debug) printf("pyics_read: %d dimensions: ", ndims);
	for (d=0; d<ndims; d++) {
		if (debug == 1) printf("%ld ", dims[d]);
		//npy_dims[d] = (npy_intp) dims[d];
		// Reverse dimensions
		npy_dims[ndims-1-d] = dims[d];
	}
	if (debug) printf("\npyics_read: Datasize: %ld\n", bufsize);
	
	// Convert datatype from ICS type to PyArray type
	switch (dt) {
		case (Ics_uint8): npy_type = PyArray_UINT8; break;
		case (Ics_sint8): npy_type = PyArray_INT8; break;
		case (Ics_uint16): npy_type = PyArray_UINT16; break;
		case (Ics_sint16): npy_type = PyArray_INT16; break;
		case (Ics_uint32): npy_type = PyArray_UINT32; break;
		case (Ics_sint32): npy_type = PyArray_INT32; break;
		case (Ics_real32): npy_type = PyArray_FLOAT32; break;
		case (Ics_real64): npy_type = PyArray_FLOAT64; break;
		case (Ics_complex32):
		case (Ics_complex64):
		case (Ics_unknown):
		default: 
			PyErr_SetString(PyExc_ValueError, "In pyics_read: datatype unknown/unsupported.");
      free(buf);
			return NULL;
	}
	
	// Create numpy array from the data 
	outdata = (PyArrayObject*) PyArray_SimpleNewFromData(ndims, npy_dims,
                                                       npy_type, (void *) buf);
	// Make sure Python owns the data, so it will free the data after use
	PyArray_FLAGS(outdata) |= NPY_OWNDATA;
	
	if (!PyArray_CHKFLAGS(outdata, NPY_OWNDATA)) {
		PyErr_SetString(PyExc_RuntimeError, "In pyics_read: unable to own the data, will cause memory leak. Aborting");
    free(buf);
		return NULL;
	}
	
	// Return the data in a dict with some metainfo attached
	// NB: Use 'N' for PyArrayObject s, because when using 'O' it will create 
	// another reference count such that the memory will never be deallocated.
	// See:
	// http://www.mail-archive.com/numpy-discussion@scipy.org/msg13354.html 
	// ([Numpy-discussion] numpy CAPI questions)
	return Py_BuildValue("{s:N,s:s}", "data", outdata, "header", "<empty>");
}


/* 
 @brief save an ICS file to disk
 @param [in] filename
 @param [in] data
 @param [in] (optional) debug flag
 @return Number of bytes written on success, NULL pointer on failure
 */
static PyObject *pyics_write(PyObject *self, PyObject *args) {
  // Parse function arguments
	char *filename;
	PyArrayObject *npydata, *npydata_align;
  int debug=0;
  
  if (!PyArg_ParseTuple(args, "sO!|i", &filename, &PyArray_Type, &npydata, &debug)) 
    return NULL;
  
  // ICS file I/O variables
  ICS *ip;
  Ics_DataType dt;
  int ndims;
  size_t dims[ICS_MAXDIM];
  size_t bufsize;
  void *npydata_ptr;
  Ics_Error retval;
  
  retval = IcsOpen(&ip, filename, "w2");
  if (retval != IcsErr_Ok) {
    PyErr_SetString(PyExc_RuntimeError, "In pyics_write: unable to open file. Aborting.");
    return NULL;
  }
  
  // Sanitize data, make a new array from the old array and force the
	// NPY_CARRAY_RO requirement which ensures a C-contiguous and aligned
	// array will be made
	// Increase reference count to the type descriptor as it is "stolen" by PyArray_FromArray
	// WARNING: anadata_align still holds a reference to anadata and must be unreferenced when we're done
  if (!PyArray_ISCARRAY(npydata)) {
    if (debug) printf("pyics_write: npy data is not a c-array, aligning data...\n");
    Py_INCREF(((PyArrayObject*) npydata)->descr);
    npydata_align = (PyArrayObject *) PyArray_FromArray(npydata,((PyArrayObject*) npydata)->descr, NPY_CARRAY_RO);
  }
  else {
    if (debug) printf("pyics_write: npy data is a c-array.\n");
    npydata_align = npydata;
  }
	
	// Get a pointer to the aligned data
	npydata_ptr = PyArray_DATA(npydata_align);
  bufsize = PyArray_NBYTES(npydata_align);
	
	// Convert datatype from PyArray type to ICS type
	switch (PyArray_TYPE((PyObject *) npydata_align)) {
		case (PyArray_UINT8): dt = Ics_uint8; break;
		case (PyArray_INT8): dt = Ics_sint8; break;
		case (PyArray_UINT16): dt = Ics_uint16; break;
		case (PyArray_INT16): dt = Ics_sint16; break;
		case (PyArray_UINT32): dt = Ics_uint32; break;
		case (PyArray_INT32): dt = Ics_sint32; break;
		case (PyArray_FLOAT32): dt = Ics_real32; break;
		case (PyArray_FLOAT64): dt = Ics_real64; break;
		default:
			PyErr_SetString(PyExc_ValueError, "In pyics_write: datatype cannot be stored as ICS file.");
			return NULL;
	}
  
  // Convert dimensions
	PyArrayObject *tmparr = (PyArrayObject*) npydata_align;
	ndims = tmparr->nd;
	npy_intp *npy_dims = PyArray_DIMS(tmparr);
	
	if (debug) printf("pyics_write: Dimensions: ");
  int d=0;
	for (d=0; d<ndims; d++) {
		dims[d] = npy_dims[ndims-1-d];
		if (debug) printf(" %lud", dims[d]);
	}
	if (debug) printf("\npyics_write: Total is %d-dimensional\n", ndims);
    
  IcsSetLayout(ip, dt, ndims, dims);
  IcsSetData(ip, npydata_ptr, bufsize);
  //IcsSetCompression(ip, IcsCompr_gzip, 6);
  IcsAddHistoryString(ip, "author", "pyics 0.2.0a");
  
  retval = IcsClose(ip);
  if (retval != IcsErr_Ok) {
    PyErr_SetString(PyExc_RuntimeError, "In pyics_write: closing file failed.");
    return NULL;
  }

  Py_DECREF(npydata_align);           // Release npydata_align
	return Py_BuildValue("i", 1);
}
