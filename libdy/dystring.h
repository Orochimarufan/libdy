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
 * @file libdy/dystring.h
 * @brief String interfaces
 * FIXME: should be called string.h, but clashes with std C <string.h>
 */

#include "types.h"
#include "config.h"
#include "object.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Strings
// strings are UTF-8 encoded.
LIBDY_API bool DyString_Check(DyObject *obj);

/**
 * @brief Create a string object from a character array and size
 * @param data The character data
 * @param size The string size
 * @return New reference to a string object
 * @sa dy_string.h
 * @sa DyString_InternStringFromStringAndSize
 */
LIBDY_API DyObject *DyString_FromStringAndSize(const char *data, size_t size);

/**
 * @brief Create a string object from a NTBS
 * @param cstr The null-terminated C string
 * @return New reference to a string object
 * @sa DyString_FromStringAndSize
 * @sa DyString_InternStringFromString
 */
static inline DyObject *DyString_FromString(const char *cstr)
{
	return DyString_FromStringAndSize(cstr, strlen(cstr));
}

/**
 * @brief Get a c-string copy of the string object
 * @param self The string object
 * @return a new NTBS buffer with the same contents as @param self
 * NOTE: buffer must be freed with free()
 */
//char *DyString_GetString(DyObject *self);

/**
 * @brief Get the buffer behind a string object
 * @param self The string object
 * @return A readonly character buffer
 * @sa DyString_GetString
 * @sa DyString_Size
 */
LIBDY_API const char *DyString_AsString(DyObject *self);


// ----------------------------------------------------------------------------
// String Interning
/**
 * @brief Check if a string object is interned and return the interned instance
 * @param str The string object to check for
 * @return Borrowed reference to the interned string object or NULL, if this string value isnt interned
 */
LIBDY_API DyObject *DyString_Interned(DyObject *str);

/**
 * @brief Check if a string is interned and return the interned object
 * @param cstr The C string to check for
 * @return Borrowed reference to the interned string object or NULL
 * @sa DyString_Interned
 */
LIBDY_API DyObject *DyString_InternedString(const char *cstr);

/**
 * @brief Intern a string object or return the interned instance
 * @param str The string object to intern
 * @return Borrowed reference to the interned string instance
 * ATTENTION: the argument is not released in case the string is already interned
 *  and the interned instance is not retained either. (Refcounts are untouched.)
 * @sa DyString_InternInplace to handle replacement refcounting
 */
LIBDY_API DyObject *DyString_Intern(DyObject *str);

/**
 * @brief Intern a string object and put the result back
 * @param strp Reference to a string object pointer.
 */
static inline void DyString_InternInplace(DyObject **strp)
{
    DyObject *interned = DyString_Intern(*strp);
    if (interned != *strp)
    {
        Dy_Retain(interned);
        Dy_Release(*strp);
    }
    *strp = interned;
}

/**
 * @brief Create an interned string object from character array and size
 * @param data The character data
 * @param size The string size
 * @return New reference to a string object
 * @sa DyString_FromStringAndSize
 */
LIBDY_API DyObject *DyString_InternStringFromStringAndSize(const char *data, size_t size);

/**
 * @brief Create an interned string object from a NTBS
 * @param cstr The null-terminated C string
 * @return New reference to a string object
 * @sa DyString_FromString
 */
static inline DyObject *DyString_InternStringFromString(const char *cstr)
{
    return DyString_InternStringFromStringAndSize(cstr, strlen(cstr));
}

#ifdef __cplusplus
}
#endif