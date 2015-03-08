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

#include "dy_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef DyObject *(*DyNoArgFn)();
typedef DyObject *(*DyNoArgFnWithData)(void *data);

typedef DyObject *(*DySimpleFn)(DyObject *arg);
typedef DyObject *(*DyArgListFnWithData)(DyObject *arglist, void *data);

typedef DyObject *(*DyArgListFn)(DyObject *arglist);
typedef DyObject *(*DySimpleFnWithData)(DyObject *arg, void *data);

DyObject *DyCall_CreateNoArg(DyNoArgFn fn);
DyObject *DyCall_CreateNoArgWithData(DyNoArgFnWithData fn, void *data);

DyObject *DyCall_CreateSimple(DySimpleFn fn);
DyObject *DyCall_CreateSimpleWithData(DySimpleFnWithData fn, void *data);

DyObject *DyCall_Create(DyArgListFn fn);
DyObject *DyCall_CreateWithData(DyArgListFnWithData fn, void *data);

#ifdef __cplusplus
}
#endif