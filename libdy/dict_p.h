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
#include "freelist_p.h"

#define DY_BLOCK_SIZE 8
#define DY_TABLE_SIZE 1

/**
 * @file dict_p.h
 * @brief Dictionary implementation header
 */
// A bucket
typedef struct bucket_t {
    Dy_hash_t hash;
    struct _DyObject *key;
    struct _DyObject *value;
    struct bucket_t *next;
} bucket_t;

// Linked List to store the blocks
typedef struct bucket_block_t {
    struct bucket_block_t *next;
    size_t size;
    freelist_struct(bucket_t, buckets);
} bucket_block_t;

// The actual object structure
typedef struct _DyDictObject {
    DyObject_HEAD

    // Simple Inheritance
    struct _DyDictObject *parent;

    // Bucket Blocks
    struct bucket_block_t *blocks;

    // The embedded table
    struct bucket_t table[DY_TABLE_SIZE];
} DyDictObject;

// Prototypes
void dict_destroy(DyDictObject *self);
int dict_clean(DyDictObject *self);
DyObject *dict_repr(DyDictObject *self);

DyObject *dict_get(DyDictObject *o, DyObject *key, Dy_hash_t hash);

DyObject *dict_getitem(DyDictObject *self, DyObject *key);
DyObject *dict_getitemu(DyDictObject *self, DyObject *key);

int dict_setitem(DyDictObject *self, DyObject *key, DyObject *value);
