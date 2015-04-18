/*
 *  Dynamic Data exchange library [libdy]
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

#include "types.h"
#include "config.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Calling
// Calls should always return new references or NULL on error.
/**
 * @brief Check if an object is callable
 * @param obj The object
 */
LIBDY_API bool DyCallable_Check(DyObject *obj);

/**
 * @brief Call a callable object with a list of arguments
 * @param self The callable object
 * @param arglist The argument list
 * @return The call result
 */
LIBDY_API DyObject *Dy_Call(DyObject *self, DyObject *arglist);

/**
 * @brief Call a callable object without arguments
 * @param self The callable object
 * @return The call result
 */
LIBDY_API DyObject *Dy_Call0(DyObject *self);

/**
 * @brief Call a callable object with a single argument
 * @param self The callable object
 * @param arg The argument
 * @return The call result
 */
LIBDY_API DyObject *Dy_Call1(DyObject *self, DyObject *arg);

/**
 * @brief Call a callable object with two arguments
 * @param self The callable object
 * @param arg1 The first argument
 * @param arg2 The second argument
 * @return The call result
 */
LIBDY_API DyObject *Dy_Call2(DyObject *self, DyObject *arg1, DyObject *arg2);

/**
 * @brief Call a callable object with three arguments
 * @param self The callable object
 * @param arg1 The first argument
 * @param arg2 The second argument
 * @param arg3 The third argument
 * @return The call result
 */
LIBDY_API DyObject *Dy_Call3(DyObject *self, DyObject *arg1, DyObject *arg2, DyObject *arg3);

// Creating callables
typedef DyObject *(*DyNoArgFn)();
typedef DyObject *(*DyNoArgFnWithData)(void *data);

typedef DyObject *(*DySimpleFn)(DyObject *arg);
typedef DyObject *(*DyArgListFnWithData)(DyObject *arglist, void *data);

typedef DyObject *(*DyArgListFn)(DyObject *arglist);
typedef DyObject *(*DySimpleFnWithData)(DyObject *arg, void *data);

LIBDY_API DyObject *DyCall_CreateNoArg(DyNoArgFn fn);
LIBDY_API DyObject *DyCall_CreateNoArgWithData(DyNoArgFnWithData fn, void *data);

LIBDY_API DyObject *DyCall_CreateSimple(DySimpleFn fn);
LIBDY_API DyObject *DyCall_CreateSimpleWithData(DySimpleFnWithData fn, void *data);

LIBDY_API DyObject *DyCall_Create(DyArgListFn fn);
LIBDY_API DyObject *DyCall_CreateWithData(DyArgListFnWithData fn, void *data);


#ifdef __cplusplus
}
#endif