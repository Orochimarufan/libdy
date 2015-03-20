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

#include "dy_host.h"

extern struct _DyHost {
    // Strings
    Dy_string_hash_fn string_hash_fn;
    // Dictionaries
    size_t dict_table_size;
    size_t dict_block_size;
    // Memory management
    Dy_MemoryManager_t mm;
} DyHost;

inline static size_t smin(size_t a, size_t b)
{
    return a > b ? b : a;
}

inline static void *dy_malloc(size_t size)
{
    return DyHost.mm.malloc(size);
}

inline static void dy_free(void *ptr)
{
    return DyHost.mm.free(ptr);
}

inline static void *dy_realloc(void *ptr, size_t size)
{
    return DyHost.mm.realloc(ptr, size);
}
