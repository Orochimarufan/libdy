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

/**
 * @file libdy/linalloc.h
 * @brief A simple linear allocator.
 *
 * Create a new pool using dy_linalloc_new(), passing it a parent allocator
 * Then it can be used through dy_linalloc_malloc() and dy_linalloc_remaining()
 * Finally, free the whole pool using the parent allocator's free()
 *
 * This is mostly useful to store larger anmounts of temporary data.
 */

#include "config.h"

#include <stdint.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif


typedef struct dy_linalloc_t dy_linalloc_t;


/**
 * @brief Create a new linear memory pool
 * @param size The pool size
 * @return A new linear memory pool
 */
LIBDY_API dy_linalloc_t *dy_linalloc_new(size_t size, void*(*malloc)(size_t));

/**
 * @brief Allocate memory from the pool
 * @param la The linalloc instance
 * @param size The anmount of memory needed
 * @return A free chunk of memory
 */
LIBDY_API void *dy_linalloc_malloc(dy_linalloc_t *la, size_t size);

/**
 * @brief Check how much memory is free in the pool
 * @param la The linalloc instance
 * @return The anmount of free memory in the pool
 */
LIBDY_API size_t dy_linalloc_remaining(dy_linalloc_t *la);


#ifdef __cplusplus
}
#endif
