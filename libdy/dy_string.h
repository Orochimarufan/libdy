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

/**
 * @file dy_string.h
 * @brief Additional functions for working with string objects
 */

// Intern
/**
 * @brief Check if a string object is interned and return the interned instance
 * @param str The string object to check for
 * @return Borrowed reference to the interned string object or NULL, if this string value isnt interned
 */
DyObject *DyString_Interned(DyObject *str);

/**
 * @brief Check if a string is interned and return the interned object
 * @param cstr The C string to check for
 * @return Borrowed reference to the interned string object or NULL
 * @sa DyString_Interned
 */
DyObject *DyString_InternedString(const char *cstr);

/**
 * @brief Intern a string object or return the interned instance
 * @param str The string object to intern
 * @return Borrowed reference to the interned string instance
 * ATTENTION: the argument is not released in case the string is already interned
 *  and the interned instance is not retained either. (Refcounts are untouched.)
 * @sa DyString_InternInplace to handle replacement refcounting
 */
DyObject *DyString_Intern(DyObject *str);

/**
 * @brief Intern a string object and put the result back
 * @param strp Reference to a string object pointer.
 */
inline void DyString_InternInplace(DyObject **strp)
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
DyObject *DyString_InternStringFromStringAndSize(const char *data, size_t size);

/**
 * @brief Create an interned string object from a NTBS
 * @param cstr The null-terminated C string
 * @return New reference to a string object
 * @sa DyString_FromString
 */
inline DyObject *DyString_InternStringFromString(const char *cstr)
{
    return DyString_InternStringFromStringAndSize(cstr, strlen(cstr));
}

#ifdef __cplusplus
}
#endif