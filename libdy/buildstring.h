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

/**
 * @file libdy/buildstring.h
 * @brief A string building utility.
 *
 * This is an API for incrementally building a string while avoiding copying
 * the whole string around large anmounts of times.
 * 
 * This is achieved by keeping metadata about the parts that need to be con-
 * catenated into the final string and only merging them when all is said
 * and done.
 * 
 * The metadata overhead possibly poses a large overhead if it is used to create
 * short strings.
 * 
 * The free_part property can be used to specify a deallocation callback.
 * It gets called when the buildstring object gets freed with dy_buildstring_free()
 * If all parts need the same deallocation procedure, dy_buildstring_free_content()
 * can be used as a shortcut to setting the individual callbacks.
 * NEVER use the latter when you don't have _full_ control over the buildstring
 * instance. Note that that is already true when you call other functions operating
 * on the buildstring that add parts you have no control over. It might cause
 * you trying to free something that's not supposed to be freed (For example,
 * a static variable) i.e only use dy_buildstring_append and dy_buildstring_printf
 * if you intend to use dy_buildstring_free_content()
 */

#pragma once

#include "types.h"
#include "config.h"

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef void(*dy_buildstring_part_free_fn)(char*);

/**
 * @brief Buildstring part node
 * @attention This might become private in the future
 */
typedef struct dy_buildstring_t {
    const char *part;                       ///< The data this part refers to
    size_t part_size;                       ///< The size of this part
    dy_buildstring_part_free_fn free_part;  ///< An optional function to deallocate the part
    struct dy_buildstring_t *next;          ///< A pointer to the next part
} dy_buildstring_t;


/**
 * @brief Create a new string builder
 * @param part The first part
 * @param size The size of the first part
 * @return A new string builder or NULL if allocation failed
 * @attention The part content is not copied, so it needs to outlive the string builder
 */
LIBDY_API dy_buildstring_t *dy_buildstring_new(const char *part, size_t size);

/**
 * @brief Append to a string builder
 * @param bs The string builder
 * @param part The part
 * @param size The part size
 * @return NULL if allocation failed
 * @attention The part content is not copied, so it needs to outlive the string builder
 */
LIBDY_API dy_buildstring_t *dy_buildstring_append(dy_buildstring_t *bs, const char *part, size_t size);

/**
 * @brief Append a sprintf-style format to the string builder
 * @param bs The string builder
 * @param fmt The format string
 * @return NULL on failure
 * @attention This will copy all input into a new buffer owned by the string builder
 */
LIBDY_API dy_buildstring_t *dy_buildstring_printf(dy_buildstring_t *bs, const char *fmt, ...);


/**
 * @brief Like append(), but automatically does the first new()
 * @param bs The string builder
 * @param part The part
 * @param size The part size
 * @return NULL if allocation failed
 * @attention The part content is not copied, so it needs to outlive the string builder
 */
LIBDY_API dy_buildstring_t *dy_buildstring_more(dy_buildstring_t **bs, const char *part, size_t size);

/**
 * @brief Retrieve the total size of a string builder
 * @param bs The string builder
 * @return It's total size
 */
LIBDY_API size_t dy_buildstring_size(dy_buildstring_t *bs);

/**
 * @brief Copy the string builder content to a character buffer
 * @param dest The destination buffer
 * @param bs The string builder
 * @return The number of bytes copied
 * @attention You probably want to add a \0 to the end of the buffer:
 *  @code size_t size = dy_buildstring_copy(buf, bs, buf_size - 1);
 *  buf[size] = 0;@endcode
 */
LIBDY_API size_t dy_buildstring_copy(char *dest, dy_buildstring_t *bs, size_t space);

/**
 * @brief Free a string builder
 * @param bs The string builder
 * @note Content isn't free'd unless free_part is set!
 */
LIBDY_API void dy_buildstring_free(dy_buildstring_t *bs);

/**
 * @brief Free a string builder and all content without a defined free_part
 * @param bs The string builder
 * @param free The default memory free function (usually free())
 * @sa dy_buildstring_free
 * @attention DO NOT USE UNLESS YOU KNOW WHAT YOU'RE DOING!
 *  Read the warning in the header description!
 */
LIBDY_API void dy_buildstring_free_content(dy_buildstring_t *bs, void(*free)(void*));


// =============================================================================
// DyString integration
// -----------------------------------------------------------------------------
/**
 * @brief Create a libdy string from a string builder
 * @param bs The string builder
 * @return A new DyObject or NULL on failure.
 * @note Sets exception on failure
 */
LIBDY_API DyObject *dy_buildstring_build(dy_buildstring_t *bs);

/**
 * @brief Append a libdy string to the string builder
 * @param bs The string builder
 * @param s The string
 * @return NULL on failure
 * @note The string builder will keep a reference to the string!
 * @note Sets exception on failure
 */
LIBDY_API dy_buildstring_t *dy_buildstring_append2(dy_buildstring_t *bs, DyObject *s);

/**
 * @brief Append an object's representation to the string builder
 * @param bs The string builder
 * @param obj The libdy object
 * @return NULL on failure
 * @note Sets exception on failure
 */
LIBDY_API dy_buildstring_t *dy_buildstring_repr(dy_buildstring_t *bs, DyObject *obj);

#ifdef __cplusplus
}
#endif
