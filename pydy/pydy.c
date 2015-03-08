/*
 *  Dynamic Data exchange library, Python extension [pydy]
 *  Copyright (C) 2015 Taeyeon Mori
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libdy/dy.h>
#include <Python.h>

static PyObject *DyError;

// Dy types
typedef struct {
    PyObject_HEAD
    DyObject *o;
} PyDy;

static PyTypeObject PyDyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pypdy.PyDy",              /* tp_name */
    sizeof(PyDy),              /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "libdy object wrapper",    /* tp_doc */
};

// Module Definition
static PyMethodDef DyMethods[] = {
    {NULL, NULL, 0, NULL}
};

static PyModuleDef Module = {
    PyModuleDef_HEAD_INIT,
    "pydy",
    "libdy python bindings",
    -1,
    DyMethods
};

PyMODINIT_FUNC
PyInit_pydy(void)
{
    PyObject *m;
    
    PyDyType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&PyDyType) < 0)
    	return NULL;
    
    m = PyModule_Create(&Module);
    if (!m)
    	return NULL;
    
    DyError = PyErr_NewException("pydy.DyError", NULL, NULL);
    Py_INCREF(DyError);
    PyModule_AddObject(m, "DyError", DyError);
    
    Py_INCREF(&PyDyType);
    PyModule_AddObject(m, "PyDy", (PyObject *)&PyDyType);
    
    return m;
}