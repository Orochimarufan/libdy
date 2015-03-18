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

#include "dy_p.h"

/**
 * @file list_p.h
 * @brief List implementation header
 */

typedef struct _DyListObject {
    DyObject_HEAD
    size_t size;
    size_t allocated;
    DyObject **items;
} DyListObject;

void list_destroy(DyListObject *self);

DyObject *list_getitem(DyListObject *self, ssize_t key);
DyObject *list_getitemu(DyListObject *self, ssize_t key);
bool list_setitem(DyListObject *self, ssize_t key, DyObject *value);
DyObject *list_repr(DyListObject *self);
