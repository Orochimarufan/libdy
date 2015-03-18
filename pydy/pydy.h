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

#pragma once

#include <Python.h>

/**
 * @file pydy.h
 * @brief PyDy header.
 * NOTE: This header is for use in embedded python interpreters.
 *       you should link your application against pydy directly and
 *       call PyInit_pydy() manually, after Py_Initialize().
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Wrap a DyObject in the python wrapper
 * @return A new Python reference
 */
PyObject *PyDy_WrapObject(DyObject *);

/**
 * @brief Wrap a DyObject in the python wrapper
 * @return A new Python reference
 * This version steals the DyObject reference in the process
 */
PyObject *PyDy_StealObject(DyObject *);

/**
 * @brief Convert a Python value to a DyObject
 * @return A new DyObject reference
 */
DyObject *PyDy_FromPython(PyObject *);

/**
 * @brief Convert a Python value to a DyObject
 * @return A new DyObject reference
 * This version steals the Python reference in the process
 */
DyObject *PyDy_StealFromPython(PyObject *);

/**
 * @brief The initialization function
 * @return A pointer to the module
 * Must be called after Py_Initialize()
 */
PyMODINIT_FUNC PyInit_pydy();

#ifdef __cplusplus
}
#endif
