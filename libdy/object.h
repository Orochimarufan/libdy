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
 * @file libdy/object.h
 * @brief Common operations on libdy objects
 */

#include "types.h"
#include "config.h"

#include <stdbool.h>
#include <stdlib.h>


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @param object The object
 * @return the object's type (see DyObject_Type)
 * Note that Undefined is DY_NONE, like None
 */
LIBDY_API DyObject_Type Dy_Type(DyObject *object);

/**
 * @brief Retrieve the name of a type in the DyObject_Type enumeration
 * @param t The type value
 */
LIBDY_API const char *  Dy_GetTypeName(DyObject_Type t);


// ----------------------------------------------------------------------------
// Reference counting
/**
 * @brief Retain (Keep) a reference to an object
 * @param self The object
 * @return The same object
 */
LIBDY_API DyObject *Dy_Retain(DyObject *self);

/**
 * @brief Release a reference to an object
 * @param self The object
 * The Object will be deleted if this was the last reference
 */
LIBDY_API void      Dy_Release(DyObject *self);

/**
 * @brief Pass a reference to an object
 * @param self The object
 * @return The same object
 * This releases the reference WITHOUT deleting the object if it reaches 0.
 * Use it to simplify inserting new objects into containers:
 *  DyObject_SetAttrString(object, "hello", Dy_Pass(DyString_FromString("Hello, World"));
 * Only use this if you are the only one holding a reference to this object! Otherwise,
 *  The object could vanish before the container references it.
 */
LIBDY_API DyObject *Dy_Pass(DyObject *self);


// ----------------------------------------------------------------------------
// Hashing
/**
 * @brief Compute the hash of an object
 * @param self The object
 * @return The hash
 * If the object cannot be hashed, an exception is set and 0 is returned
 * @sa Dy_HashEx
 */
LIBDY_API Dy_hash_t Dy_Hash(DyObject *self);

/**
 * @brief Compute the hash of an object
 * @param self The object
 * @param out Variable to store the result in
 * @return Whether the object could be hashed
 */
LIBDY_API bool      Dy_HashEx(DyObject *self, Dy_hash_t *out);

// Comparison
LIBDY_API bool      Dy_Equals(DyObject *self, DyObject *other);

// Representation
/**
 * @brief Get the string representation of an object
 * @param self The object
 * NOTE: this will break on reference cycles!
 */
LIBDY_API DyObject *Dy_Repr(DyObject *self);

/**
 * @brief Convert the object to a string
 * @param self The object
 * In most cases, this is equivalent to Dy_Repr, and thus shares it's inability
 * to handle reference cycles.
 * @sa Dy_Repr
 */
LIBDY_API DyObject *Dy_Str(DyObject *self);

/**
 * @brief Get the string representation
 * @param self The object
 * @return A static buffer with the representation.
 * Note: the buffer is only 1024 characters long.
 *     	Use Dy_Repr() and DyString_*() directly if that isn't enough.
 */
LIBDY_API const char *Dy_AsRepr(DyObject *self);

// Length
/**
 * @brief Get an object's length
 * @arg self The object
 * @return The length
 * For lists: the number of items contained
 * For strings: the string length
 */
LIBDY_API size_t      Dy_Length(DyObject *self);


#ifdef __cplusplus
}
#endif
