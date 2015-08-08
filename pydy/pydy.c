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

#include <Python.h>
#include <libdy/dy.h>
#include <libdy/exceptions.h>
#include <libdy/userdata.h>
#include "pydy.h"

static PyObject *DyError;

// Prototypes
static void raise_dy_error();

static DyObject *py2dy(PyObject *o);
static PyObject *dy2py(DyObject *obj);
static PyObject *py2pydy(PyObject *o);
static PyObject *dy2pyreturn(DyObject *o);

static DyObject *pytuple2dylist(PyObject *o);
static PyObject *dystr2pyunicode(DyObject *obj);
static PyObject *dylong2pylong(DyObject *obj);
static PyObject *dyfloat2pyfloat(DyObject *obj);

static PyObject *PyDy_New(DyObject *obj);
static PyObject *PyDy_Pass(DyObject *obj);
static bool PyDy_Check(PyObject *o);
static bool PyDy_Set(PyObject *self, DyObject *obj);
static DyObject *PyDy_Get(PyObject *self);

// Dy types
typedef struct PyDy {
    PyObject_HEAD
    DyObject *obj;
} PyDy;

static int dy_bool(PyDy *self)
{
    if (self->obj == Dy_None || self->obj == Dy_False)
        return 0;
    else if (self->obj == Dy_True)
        return 1;
    else if (Dy_Type(self->obj) == DY_LONG)
        return DyLong_Get(self->obj) != 0;
    else if (Dy_Type(self->obj) == DY_FLOAT)
        return DyFloat_Get(self->obj) != 0.0;
    else if (DyList_Check(self->obj) || DyString_Check(self->obj))
        return Dy_Length(self->obj) > 0;
    else
        return 0; // TODO: other ones
}

static PyObject *dy_long(PyDy *self)
{
    if (Dy_Type(self->obj) == DY_LONG)
        return dylong2pylong(self->obj);
    //else if (Dy_Type(self) == DY_FLOAT)
    //    convert to double
    else
    {
        PyErr_Format(PyExc_TypeError, "PyDy Object of type %s cannot be converted to an integer", Dy_GetTypeName(Dy_Type(self->obj)));
        return NULL;
    }
}

static PyObject *dy_float(PyDy *self)
{
    if (Dy_Type(self->obj) == DY_FLOAT)
        return dyfloat2pyfloat(self->obj);
    // else if (Dy_Type(self) == DY_LONG)
    //    convert to long
    else
    {
        PyErr_Format(PyExc_TypeError, "PyDy Object of type %s cannot be converted to a floating number", Dy_GetTypeName(Dy_Type(self->obj)));
        return NULL;
    }
}

// Number protocol
static PyNumberMethods PyDy_NumberMethods = {
    0,              /* nb_add */
    0,              /* nb_subtract */
    0,              /* nb_multiply */
    0,              /* nb_remainder */
    0,              /* nb_divmod */
    0,              /* nb_power */
    0,              /* nb_negative */
    0,              /* nb_positive */
    0,              /* nb_absolute */
    dy_bool,        /* nb_bool */
    0,              /* nb_invert */
    0,              /* nb_lshift */
    0,              /* nb_rshift */
    0,              /* nb_and */
    0,              /* nb_xor */
    0,              /* nb_or */
    dy_long,        /* nb_int */
    0,              /* nb_reserved reserved */
    dy_float,       /* nb_float */
};

static PyObject *dy_getitem(PyDy *self, PyObject *key)
{
    DyObject *key_obj = py2dy(key);
    if (!key_obj)
        return NULL;

    DyObject *val_obj = Dy_GetItem(self->obj, key_obj);
    if (!val_obj)
    {
        raise_dy_error();
        Dy_Release(key_obj);
        return NULL;
    }

    Dy_Release(key_obj);

    return PyDy_Pass(val_obj);
}

static int dy_setitem(PyDy *self, PyObject *key, PyObject *value)
{
    DyObject *key_obj = py2dy(key);
    if (!key_obj)
        return -1;

    DyObject *val_obj = py2dy(value);
    if (!val_obj)
    {
        Dy_Release(val_obj);
        return -1;
    }
    
    if (!Dy_SetItem(self->obj, key_obj, val_obj))
    {
        raise_dy_error();
        Dy_Release(key_obj);
        Dy_Release(val_obj);
        return -1;
    }

    Dy_Release(key_obj);
    Dy_Release(val_obj);
    return 0;
}

static PyMappingMethods PyDy_MappingMethods = {
    0,              /* mp_length */
    dy_getitem,     /* mp_subscript */
    dy_setitem,     /* mp_ass_subscript */
};

static Py_ssize_t dy_length(PyDy *self)
{
    size_t length = Dy_Length(self->obj);
    if (length == 0 && DyErr_Occurred())
    {
        raise_dy_error();
        return -1;
    }
    return length;
}

static PyObject *dy_getitemlong(PyDy *self, Py_ssize_t ix)
{
    DyObject *obj = Dy_GetItemLong(self->obj, ix);
    if (!obj)
    {
        raise_dy_error();
        return NULL;
    }

    return PyDy_Pass(obj);
}

static int dy_setitemlong(PyDy *self, Py_ssize_t ix, PyObject *value)
{
    DyObject *val_obj = py2dy(value);
    if (!val_obj)
        return -1;

    if (!Dy_SetItemLong(self->obj, ix, val_obj))
    {
        raise_dy_error();
        Dy_Release(val_obj);
        return -1;
    }

    Dy_Release(val_obj);
    return 0;
}

static PySequenceMethods PyDy_SequenceMethods = {
    dy_length,      /* sq_length */
    0,              /* sq_concat */
    0,              /* sq_repeat */
    dy_getitemlong, /* sq_item */
    dy_setitemlong, /* sq_ass_item */
    0,              /* sq_contains */
    0,              /* sq_inplace_repeat */
};

static PyObject *dy_str(PyDy *self)
{
    DyObject *str = Dy_Str(self->obj);
    if (!str)
    {
        raise_dy_error();
        return NULL;
    }

    PyObject *pys = dystr2pyunicode(str);
    Dy_Release(str);
    return pys;
}

static PyObject *dy_repr(PyDy *self)
{
    DyObject *str = Dy_Repr(self->obj);
    if (!str)
    {
        raise_dy_error();
        return NULL;
    }

    PyObject *pys = dystr2pyunicode(str);
    Dy_Release(str);
    return pys;
}

static PyObject *dy_call(PyDy *self, PyObject *args, PyObject *kwds)
{
    if (!DyCallable_Check(self->obj))
    {
        PyErr_SetString(PyExc_TypeError, "PyDy object not callable!");
        return NULL;
    }
    if (kwds)
    {
        PyErr_SetString(PyExc_TypeError, "PyDy callables do not support keyword arguments!");
        return NULL;
    }

    DyObject *dyargs = pytuple2dylist(args);
    if (!dyargs)
        return NULL;

    DyObject *result = DyCallable_Call(self->obj, NULL, dyargs); // TODO: self
    Dy_Release(dyargs);
    if (!result)
    {
        raise_dy_error();
        return NULL;
    }

    return dy2pyreturn(result);
}

static void dy_finalize(PyDy *self)
{
    if (self->obj)
        Dy_Release(self->obj);
}

static PyTypeObject PyDyType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pydy.PyDy",               /* tp_name */
    sizeof(PyDy),              /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print reserved */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved reserved */
    dy_repr,                   /* tp_repr */
    &PyDy_NumberMethods,       /* tp_as_number */
    &PyDy_SequenceMethods,     /* tp_as_sequence */
    &PyDy_MappingMethods,      /* tp_as_mapping */
    0,                         /* tp_hash  */
    dy_call,                   /* tp_call */
    dy_str,                    /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "libdy object wrapper",    /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    0,                         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,                         /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
    0,                         /* tp_bases */
    0,                         /* tp_mro */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0,                         /* tp_weaklist */
    0,                         /* tp_del */
    0,                         /* tp_version_tag */
    dy_finalize,               /* tp_finalize */
};

static inline bool PyDy_Check(PyObject *o)
{
    return PyObject_IsInstance(o, (PyObject*)&PyDyType);
}

static bool PyDy_Set(PyObject *o, DyObject *dy)
{
    if (!PyDy_Check(o))
        return false;
    PyDy *d = (PyDy *) o;
    if (dy)
        Dy_Retain(dy);
    if (d->obj)
        Dy_Release(d->obj);
    d->obj = dy;
    return true;
}

static DyObject *PyDy_Get(PyObject *o)
{
    if (!PyDy_Check(o))
        return NULL;
    return ((PyDy*)o)->obj;
}

static PyObject *PyDy_New(DyObject *dyobj)
{
    PyObject *obj = PyObject_CallObject((PyObject *) &PyDyType, NULL);
    if (!obj)
        return NULL;

    if (!PyDy_Set(obj, dyobj))
    {
        Py_DECREF(obj);
        return NULL;
    }
    return obj;
}

static PyObject *PyDy_Pass(DyObject *dyobj)
{
    PyObject *obj = PyDy_New(dyobj);
    Dy_Release(dyobj);
    return obj;
}

typedef struct {
    PyObject *tp;
    PyObject *exc;
    PyObject *tb;
} PyDy_errstruct;

static void PyDy_errstruct_free(void *data)
{
    PyDy_errstruct *es = (PyDy_errstruct*)data;
    Py_XDECREF(es->tp);
    Py_XDECREF(es->exc);
    Py_XDECREF(es->tb);
    free(es);
}

static void raise_dy_error()
{
    DyObject *dy_exc = DyErr_Occurred();
    if (!dy_exc)
    {
        PyErr_SetString(PyExc_RuntimeError, "pydy: raise_dy_error called without active exception!");
        return;
    }

    if (DyErr_Filter(dy_exc, "py.Exception.esx"))
    {
        PyDy_errstruct *es = (PyDy_errstruct*)DyErr_Data(dy_exc);
        Py_XINCREF(es->tp);
        Py_XINCREF(es->exc);
        Py_XINCREF(es->tb);
        PyErr_Restore(es->tp, es->exc, es->tb);
        DyErr_Clear();
        return;
    }

    PyObject *exc = PyDy_New(dy_exc);
    if (!exc)
    {
        PyErr_SetString(PyExc_RuntimeError, "pydy: raise_dy_error failed to wrap exception object!");
        return;
    }

    PyErr_SetObject(DyError, exc);

    Py_DECREF(exc);

    DyErr_Clear();
}

static void raise_python_error_in_dy()
{
    PyObject *tp;
    PyObject *exc;
    PyObject *tb;

    PyErr_Fetch(&tp, &exc, &tb);
    if (!tp)
    {
        DyErr_Set("py.NoErrorSet", "raise_python_error_in_dy() called without python exception");
        //_BREAK_
    }
    PyErr_NormalizeException(&tp, &exc, &tb);

    if (tp == DyError)
    {
        PyObject *args = PyObject_GetAttrString(exc, "args");
        if (args)
        {
            PyObject *dyerrx = PySequence_GetItem(args, 1);
            if (dyerrx)
            {
                DyObject *dyerr = PyDy_Get(dyerrx);
                if (dyerr)
                {
                    Py_DECREF(exc);
                    DyErr_SetObject(dyerr);
                    PyErr_Clear();
                    return;
                }
            }
        }
    }

    PyDy_errstruct *es = malloc(sizeof(PyDy_errstruct));
    es->tp = tp;
    es->exc = exc;
    es->tb = tb;

    DyObject *ex;

    PyObject *str = PyObject_Str(exc);
    if (str)
    {
        ex = DyErr_Format("py.Exception.esx", "A python exception occured: %s", PyUnicode_AsUTF8(str));
        Py_DECREF(str);
    }
    else
        ex = DyErr_Set("py.Exception.esx", "A python exception occured.");

    DyErr_SetExceptionData(ex, es, &PyDy_errstruct_free);
}

// Conversion
static PyObject *parent_string = NULL;

static DyObject *pydict2dydict(PyObject *dict)
{
    DyObject *obj;
    int result;

    if (!parent_string)
        parent_string = PyUnicode_FromString("__parent__");

    result = PyDict_Contains(dict, parent_string);
    if (result < 0)
        return NULL;
    else if (result)
    {
        PyObject *parent = PyDict_GetItem(dict, parent_string);
        if (!parent)
            return NULL;

        DyObject *parent_o = py2dy(parent);
        if (!parent_o)
        {
            raise_dy_error();
            return NULL;
        }

        else if (!DyDict_Check(parent_o))
        {
            PyErr_SetString((PyObject*)&PyExc_TypeError, "DyDict parent needs to be dict too!");
            Dy_Release(parent_o);
            return NULL;
        }

        obj = DyDict_NewWithParent(parent_o);
        Dy_Release(parent_o);
    }
    else
        obj = DyDict_New();

    if (!obj)
    {
        raise_dy_error();
        return NULL;
    }

    Py_ssize_t pos = 0;
    PyObject *key;
    PyObject *value;

    while(PyDict_Next(dict, &pos, &key, &value))
    {
        DyObject *key_obj = py2dy(key);
        if (!key_obj)
        {
            Dy_Release(obj);
            return NULL;
        }

        DyObject *val_obj = py2dy(value);
        if (!val_obj)
        {
            Dy_Release(obj);
            Dy_Release(key_obj);
            return NULL;
        }

        if (!Dy_SetItem(obj, key_obj, val_obj))
        {
            Dy_Release(obj);
            Dy_Release(key_obj);
            Dy_Release(val_obj);
            raise_dy_error();
            return NULL;
        }

        Dy_Release(key_obj);
        Dy_Release(val_obj);
    }

    return obj;
}

static DyObject *pylong2dylong(PyObject *lo)
{
    DyObject *obj = DyLong_New(PyLong_AsLong(lo));
    if (!obj)
    {
        raise_dy_error();
        return NULL;
    }
    return obj;
}

static DyObject *pyunicode2dystring(PyObject *uni)
{
    Py_ssize_t size;
    char * buf = PyUnicode_AsUTF8AndSize(uni, &size);
    if (!buf)
        return NULL;

    DyObject *obj = DyString_FromStringAndSize(buf, size);
    if (!obj)
    {
        raise_dy_error();
        return NULL;
    }

    return obj;
}

static DyObject *pylist2dylist(PyObject *lst)
{
    Py_ssize_t size = PyList_Size(lst);
    DyObject *obj = DyList_NewEx(size);

    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject *itm = PyList_GetItem(lst, i);
        if (!itm)
        {
            Dy_Release(obj);
            return NULL;
        }

        DyObject *ditm = py2dy(itm);
        if (!ditm)
        {
            Dy_Release(obj);
            return NULL;
        }

        if (!DyList_Append(obj, ditm))
        {
            raise_dy_error();
            Dy_Release(ditm);
            Dy_Release(obj);
            return NULL;
        }

        Dy_Release(ditm);
    }

    return obj;
}

static DyObject *pytuple2dylist(PyObject *lst)
{
    Py_ssize_t size = PyTuple_Size(lst);
    DyObject *obj = DyList_NewEx(size);

    for (Py_ssize_t i = 0; i < size; ++i)
    {
        PyObject *itm = PyTuple_GetItem(lst, i);
        if (!itm)
        {
            Dy_Release(obj);
            return NULL;
        }

        DyObject *ditm = py2dy(itm);
        if (!ditm)
        {
            Dy_Release(obj);
            return NULL;
        }

        if (!DyList_Append(obj, ditm))
        {
            raise_dy_error();
            Dy_Release(ditm);
            Dy_Release(obj);
            return NULL;
        }

        Dy_Release(ditm);
    }

    return obj;
}

static DyObject *pyfloat2dyfloat(PyObject *flt)
{
    double value = PyFloat_AsDouble(flt);
    if (value == -1.0 && PyErr_Occurred())
        return NULL;

    DyObject *obj = DyFloat_New(value);
    if (!obj)
    {
        raise_dy_error();
        return NULL;
    }
    return obj;
}

static DyObject *py2dy(PyObject *o)
{
    DyObject *res;
    if (o == Py_None)
        res = Dy_Retain(Dy_None);
    else if (o == Py_True)
        res = Dy_Retain(Dy_True);
    else if (o == Py_False)
        res = Dy_Retain(Dy_False);
    else if (PyDict_Check(o))
        res = pydict2dydict(o);
    else if (PyDy_Check(o))
        res = Dy_Retain(PyDy_Get(o));
    else if (PyLong_Check(o))
        res = pylong2dylong(o);
    else if (PyFloat_Check(o))
        res = pyfloat2dyfloat(o);
    else if (PyUnicode_Check(o))
        res = pyunicode2dystring(o);
    else if (PyList_Check(o))
        res = pylist2dylist(o);
    else if (PyTuple_Check(o))
        res = pytuple2dylist(o);
    else
    {
        PyErr_SetString(PyExc_TypeError, "Unknown conversion.");
        return NULL;
    }

    Py_DECREF(o);
    return res;
}

static PyObject *dystr2pyunicode(DyObject *str)
{
    size_t size = Dy_Length(str);
    return PyUnicode_FromStringAndSize(DyString_AsString(str), size);
}

static PyObject *dylong2pylong(DyObject *lng)
{
    return PyLong_FromLong(DyLong_Get(lng));
}

static PyObject *dyfloat2pyfloat(DyObject *flt)
{
    return PyFloat_FromDouble(DyFloat_Get(flt));
}

static PyObject *dylist2pytuple_flat(DyObject *lst)
{
    // Raise python exc
    size_t s = Dy_Length(lst);
    PyObject *tup = PyTuple_New(s);

    if (!tup)
        return NULL;

    for (size_t i = 0; i < s; ++i)
        PyTuple_SET_ITEM(tup, i, PyDy_Pass(Dy_GetItemLong(lst, i)));

    return tup;
}

static PyObject *dy2pyreturn(DyObject *o)
{
    PyObject *res;
    if (o == Dy_None)
        res = Py_None;
    else if (o == Dy_True)
        res = Py_True;
    else if (o == Dy_False)
        res = Py_False;
    else
    {
        return PyDy_Pass(o);
    }

    Py_INCREF(res);
    Dy_Release(o);
    return res;
}

// functions
static DyObject *pyfunc_wrapper(DyObject *self, void *data, DyObject *arglist)
{
    PyObject *fn = (PyObject *)data;

    // __get__ the function object
    PyObject *meth;
    if (self)
    {
        PyObject *pyself = PyDy_New(self);
        meth = PyObject_CallMethod(fn, "__get__", "O", pyself);
        Py_DECREF(pyself);
        if (!meth)
        {
            raise_python_error_in_dy();
            return NULL;
        }
    }
    else
    {
        meth = fn;
        Py_INCREF(fn);
    }

    // Prepare arguments
    PyObject *args = dylist2pytuple_flat(arglist);
    if (!args)
    {
        raise_python_error_in_dy();
        Py_DECREF(meth);
        return NULL;
    }

    // Call function
    PyObject *result = PyObject_CallObject(meth, args);

    // Clean up
    Py_DECREF(args);
    Py_DECREF(meth);

    if (!result)
    {
        raise_python_error_in_dy();
        return NULL;
    }

    DyObject *res = py2dy(result);
    Py_DECREF(result);
    return res;
}

static void pyfunc_destructor(void *data)
{
    Py_DECREF((PyObject*)data);
}

static DyObject *pyfunc2udata(PyObject *fn)
{
    DyObject *udata = DyUser_CreateCallable(&pyfunc_wrapper, fn);
    if (!udata)
        return NULL;
    DyUser_SetDestructor(udata, &pyfunc_destructor);
    return udata;
}

// Public
DyObject *PyDy_FromPython(PyObject *o)
{
    DyObject *obj = py2dy(o);
    if (!obj)
        raise_python_error_in_dy();
    return obj;
}

DyObject *PyDy_StealFromPython(PyObject *o)
{
    DyObject *obj = PyDy_FromPython(o);
    Py_DECREF(o);
    return obj;
}

PyObject *PyDy_WrapObject(DyObject *obj)
{
    PyObject *o = PyDy_New(obj);
    if (!o)
        raise_python_error_in_dy();
    return o;
}

PyObject *PyDy_StealObject(DyObject *obj)
{
    PyObject *o = PyDy_New(obj);
    if (!o)
        raise_python_error_in_dy();
    Dy_Release(obj);
    return o;
}

// Module Definition
static PyObject *pydy_new(PyObject *self, PyObject *o)
{
    DyObject *dy = py2dy(o);
    if (!dy)
        return NULL;

    PyObject *w = PyDy_New(dy);
    Dy_Release(dy);

    if (!w)
        return NULL;
    else
        return w;
}

static PyObject *pydy_dict(PyObject *self, PyObject *args)
{
    PyObject *parent = NULL;
    if (!PyArg_ParseTuple(args, "|O!:pydy.dict", &PyDyType, &parent))
        return NULL;

    DyObject *o;
    if (parent)
        o = DyDict_NewWithParent(PyDy_Get(parent));
    else
        o = DyDict_New();

    return PyDy_Pass(o);
}

static PyObject *pydy_list(PyObject *self, PyObject *args)
{
    int size_hint = 0;
    if (!PyArg_ParseTuple(args, "|k:pydy.list", &size_hint))
        return NULL;

    DyObject *o;
    if (size_hint)
        o = DyList_NewEx(size_hint);
    else
        o = DyList_New();

    return PyDy_Pass(o);
}

static PyObject *pydy_func(PyObject *self, PyObject *fn)
{
    if (!PyCallable_Check(fn))
    {
        PyErr_SetString(PyExc_TypeError, "First argument to pydy.func needs to be a python callable!");
        return NULL;
    }

    return PyDy_Pass(pyfunc2udata(fn));
}

static PyMethodDef DyMethods[] = {
    {"new", pydy_new, METH_O, "Create PyDy object from Pyton object."},
    {"dict", pydy_dict, METH_VARARGS, "Create a PyDy dictionary, optionally with parent"},
    {"list", pydy_list, METH_VARARGS, "Create a PyDy list, optionally pre-allocating space"},
    {"func", pydy_func, METH_O, "Create a PyDy callable from a python function"},
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
    //PyModule_AddObject(m, "PyDy", (PyObject *)&PyDyType);

    return m;
}
