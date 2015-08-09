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

// String
#pragma once
#include "dy_p.h"

// Flags
#define DYSTRING_INTERNED 1
#define DYSTRING_HASH 2

// Data structure
typedef struct _DyStringObject {
    DyObject_HEAD;
    uint32_t size;
    uint8_t flags;
    DyHash hash;
    char data[1];
} DyStringObject;

// Prototypes
bool DyString_Equals(DyStringObject *, DyStringObject *);

DyStringObject *string_new(const char *s, size_t size);
DyStringObject *string_new_ex(size_t size);

void string_unintern(DyStringObject *);
void string_destroy(DyStringObject *self);

DyHash string_hash(DyStringObject *self);

struct dy_buildstring_t *string_bsrepr(struct dy_buildstring_t *bs, DyStringObject *self);
