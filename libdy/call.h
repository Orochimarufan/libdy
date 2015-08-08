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

// Checking if an object is callable
LIBDY_API bool DyCallable_Check(DyObject *obj);


/* Calling objects
 *
 * Three sets of functions exist for calling objects:
 *
 * 1) Dy_Call(self, member, ...) family:
 *      Calls 'self's member 'member'
 *      'self' must therefore be a subscriptable object
 * 2) Dy_CallString(self, member, ...) family:
 *      Short for Dy_Call(self, DyString_FromString(member), ...)
 * 3) DyCallable_Call(cal, self, ...) family:
 *      Calls the callable object 'cal'
 *      NULL may be passed as 'self'.
 */
LIBDY_API DyObject *Dy_Call(DyObject *self, DyObject *member, DyObject *arglist);
LIBDY_API DyObject *Dy_Call0(DyObject *self, DyObject *member);
LIBDY_API DyObject *Dy_Call1(DyObject *self, DyObject *member, DyObject *arg);

LIBDY_API DyObject *Dy_CallString(DyObject *self, const char *member, DyObject *arglist);
LIBDY_API DyObject *Dy_CallString0(DyObject *self, const char *member);
LIBDY_API DyObject *Dy_CallString1(DyObject *self, const char *member, DyObject *arg);

LIBDY_API DyObject *DyCallable_Call(DyObject *cal, DyObject *self, DyObject *arglist);
LIBDY_API DyObject *DyCallable_Call0(DyObject *cal, DyObject *self);
LIBDY_API DyObject *DyCallable_Call1(DyObject *cal, DyObject *self, DyObject *arg);


#ifdef __cplusplus
}
#endif
