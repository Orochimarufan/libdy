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

#include "dy_callable.h"
#include "dy_p.h"


#define DCD_NODATA 0
#define DCD_HASDATA 1

#define DCT_NOARG 0
#define DCT_SIMPLE 1
#define DCT_ARGLIST 10


typedef struct _DyCallableObject {
    DyObject_HEAD
    uint8_t flags; // "type" is already in use in the base DyObject!
    DyObject *(*function)();
    void *data;
} DyCallableObject;

