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


// FIXME: make this private?
typedef struct dy_buildstring_t {
    const char *part;
    size_t part_size;
    struct dy_buildstring_t *next;
} dy_buildstring_t;

/**
 * @brief Create a new string builder
 * @param part The first part
 * @param size The size of the first part
 * @return A new string builder or NULL if allocation failed
 * NOTE: The part content is not copied, so it needs to outlive the string builder
 */
dy_buildstring_t *dy_buildstring_new(const char *part, size_t size);

/**
 * @brief Append to a string builder
 * @param bs The string builder
 * @param part The part
 * @param size The part size
 * @return NULL if allocation failed
 * NOTE: The part content is not copied, so it needs to outlive the string builder
 */
dy_buildstring_t *dy_buildstring_append(dy_buildstring_t *bs, const char *part, size_t size);

/**
 * @brief Retrieve the total size of a string builder
 * @param bs The string builder
 * @return It's total size
 */
size_t dy_buildstring_size(dy_buildstring_t *bs);

/**
 * @brief Copy the string builder content to a character buffer
 * @param dest The destination buffer
 * @param bs The string builder
 * @return The number of bytes copied
 * NOTE: you probably want to add a \0 to the end of the buffer:
 *  size_t size = dy_buildstring_copy(buf, bs, buf_size - 1);
 *  buf[size] = 0;
 */
size_t dy_buildstring_copy(const char *dest, dy_buildstring_t *bs, size_t space);

/**
 * @brief Create a libdy string from a string builder
 * @param bs The string builder
 * @return A new DyObject or NULL on failure.
 * NOTE: This is the only function in this file that sets an Exception on failure!
 */
DyObject *dy_buildstring_build(dy_buildstring_t *bs);

/**
 * @brief Free a string builder
 * @param bs The string builder
 * NOTE: This doesn't free the actual content.
 */
void dy_buildstring_free(dy_buildstring_t *bs);

/**
 * @brief Free a string builder and its content
 * @param bs The string builder
 * @param free The memory free function (usually free())
 * @sa dy_buildstring_free
 */
void dy_buildstring_free_content(dy_buildstring_t *bs, void(*free)(void*));

#ifdef __cplusplus
}
#endif