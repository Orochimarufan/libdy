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

// Private header
#pragma once

#include "dy.h"

#include <stdlib.h>
#include <memory.h>


// Primitive new
#define /*T**/ NEW(/*typename*/ T)\
    memset(malloc(sizeof(T)), 0, sizeof(T))
#define /*T**/ NEWN(/*typename*/ T, /*int*/ N)\
    memset(malloc(sizeof(T)*N), 0, sizeof(T)*N)

// Opaque Object Header
#define DyObject_HEAD\
    uint8_t type;\
    _Atomic(unsigned long) refcnt;

struct _DyObject {
    DyObject_HEAD
};

// Private Prototypes
void Dy_InitObject(DyObject *, DyObject_Type);
void Dy_FreeObject(DyObject *);
bool Dy_HashEx(DyObject *, Dy_hash_t *);

// cause it doesn't go anywhere else either
void exception_destroy(DyObject *exc);
