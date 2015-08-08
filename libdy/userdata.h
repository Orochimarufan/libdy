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

// TODO: Fix documentation

// Checking for Userdata
LIBDY_API bool DyUserdata_Check(DyObject *obj);

/* Simple, non-callable Userdata
 *
 * Simple Userdata is used to proxy raw pointers through a libdy structure.
 *
 * The Userdata name can be used by the user to identify the type of pointer it
 * contains.
 */
LIBDY_API DyObject *DyUser_Create(void *data);
LIBDY_API DyObject *DyUser_CreateNamed(void *data, const char *name);

LIBDY_API const char *DyUser_GetName(DyObject *self);
LIBDY_API void *DyUser_GetData(DyObject *self);

typedef void (*DyUser_Destructor)(void *data);
LIBDY_API bool DyUser_SetDestructor(DyObject *ud, DyUser_Destructor fn);

/* Callables
 * 
 * Callables are created using the DyUser_CreateCallable[01](callback, data)
 * functions. All callbacks receive the object the callable was retrieved from
 * (provided it wasn't called stand-alone in which case this will be NULL)
 * as the first argument, as well as the data pointer associated with the Userdata
 * as the second argument.
 * 
 * The 1-argument variant receives the provided argument as its third argument
 * while the variable-arguments variant receives a list of provided arguments
 * as its third argument.
 * 
 */
typedef DyObject *(*DyUser_Callback)(DyObject *self, void *data, DyObject *arglist);
typedef DyObject *(*DyUser_Callback0)(DyObject *self, void *data);
typedef DyObject *(*DyUser_Callback1)(DyObject *self, void *data, DyObject *arg);

LIBDY_API DyObject *DyUser_CreateCallable(DyUser_Callback fn, void *data);
LIBDY_API DyObject *DyUser_CreateCallable0(DyUser_Callback0 fn, void *data);
LIBDY_API DyObject *DyUser_CreateCallable1(DyUser_Callback1 fn, void *data);

LIBDY_API DyObject *DyUser_CreateCallback(void(*fn)());

#ifdef __cplusplus
}
#endif
