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

#ifdef __cplusplus
extern "C" {
#endif

#include "dy.h"

#include <stdlib.h>

/**
 * @file dy_host.h
 * @brief Managing libdy internals
 *
 * These functions can be used to modify libdy behaviour like hash functions
 *  and memory allocation functions.
 * In a plugin architecture, only the plugin "host" should ever use them.
 *
 * PLEASE NOTE that changing any of these after starting to use libdy
 *  objects WILL cause A LOT OF PROBLEMS. DON'T DO IT!
 */

typedef Dy_hash_t (*Dy_string_hash_fn)(const char *data, size_t length);

/**
 * @brief Specify the hash function used for strings
 * @param func The hash function
 * This must be done before using ANY libdy objects, as it modifies
 * behavior of string and mapping objects.
 */
void DyHost_SetHashFunc(Dy_string_hash_fn func);

// Predefined hash functions:
Dy_hash_t Dy_hash_fnv1(const char *data, size_t length);
Dy_hash_t Dy_hash_Murmur3_32(const char *data, size_t length);

// Memory management
typedef void*(*Dy_malloc_fn)(size_t);
typedef void(*Dy_free_fn)(void*);
typedef void*(*Dy_realloc_fn)(void*, size_t);

typedef struct Dy_MemoryManager_t {
    Dy_malloc_fn malloc;
    Dy_free_fn free;
    Dy_realloc_fn realloc;
} Dy_MemoryManager_t;

void DyHost_SetMemoryManager(Dy_MemoryManager_t mm);

#ifdef __cplusplus
}
#endif
