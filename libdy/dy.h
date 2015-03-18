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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// ssize_t
#include <sys/types.h>

// DyObject and DyObject_Type
#include "dy_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @param object The object
 * @return the object's type (see DyObject_Type)
 * Note that Undefined is DY_NONE, like None
 */
DyObject_Type Dy_Type(DyObject *object);

const char *Dy_GetTypeName(DyObject_Type t);


// ----------------------------------------------------------------------------
// Reference counting
/**
 * @brief Retain (Keep) a reference to an object
 * @param self The object
 * @return The same object
 */
DyObject *Dy_Retain(DyObject *self);

/**
 * @brief Release a reference to an object
 * @param self The object
 * The Object will be deleted if this was the last reference
 */
void      Dy_Release(DyObject *self);

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
DyObject *Dy_Pass(DyObject *self);


// ----------------------------------------------------------------------------
// Item subscription interface (lists & dictionaries)
/**
 * @brief Retrieve an index from an object/list
 * @param self The object
 * @param key The key
 * @return A borrowed reference to the object stored in \c self for \c key
 * Throws [dy.KeyError] when \c key isn't in \c self
 */
DyObject *Dy_GetItem(DyObject *self, DyObject *key);
DyObject *Dy_GetItemString(DyObject *self, const char *key);
DyObject *Dy_GetItemLong(DyObject *self, long key);

/**
 * @brief Retrieve an index from a dictionary/list
 * @param self The subscriptable object
 * @param key The key to look up
 * @param def A fallback value
 * @return A borrowed reference to the object || \c def
 */
DyObject *Dy_GetItemD(DyObject *self, DyObject *key, DyObject *def);
DyObject *Dy_GetItemStringD(DyObject *self, const char *key, DyObject *def);
DyObject *Dy_GetItemLongD(DyObject *self, long key, DyObject *def);

/**
 * @brief Retrieve an index from a dictionary/list
 * @param self The subscriptable object
 * @param key The key to look up
 * @return A borrowed reference to the object || Dy_Undefined
 * Short for Dy_GetItemD(self, key, Dy_Undefined)
 */
DyObject *Dy_GetItemU(DyObject *self, DyObject *key);
DyObject *Dy_GetItemStringU(DyObject *self, const char *key);
DyObject *Dy_GetItemLongU(DyObject *self, long key);

/**
 * @brief Set an item in an object/list
 * @param self The object
 * @param key The key
 * @param value the new value
 * @return 
 * If value is NULL, delete the key instead
 */
bool      Dy_SetItem(DyObject *self, DyObject *key, DyObject *value);
bool      Dy_SetItemString(DyObject *self, const char *key, DyObject *value);
bool      Dy_SetItemLong(DyObject *self, long key, DyObject *value);

/**
 * @brief Check if an object/list contains a specific item
 * @param self The object
 * @param key The key
 * @return whether \c key exists in \c self
 * NOTIMPLEMENTED
 */
bool      Dy_Contains(DyObject *self, DyObject *key);
bool      Dy_ContainsString(DyObject *self, const char *key);
bool      Dy_ContainsLong(DyObject *self, long key);


// ----------------------------------------------------------------------------
// Hashing
Dy_hash_t Dy_Hash(DyObject *self);
bool Dy_HashEx(DyObject *self, Dy_hash_t *out);

// Comparison
bool      Dy_Equals(DyObject *self, DyObject *other);

// Representation
DyObject *Dy_Repr(DyObject *self);
DyObject *Dy_Str(DyObject *self);

/**
 * @brief Get the string representation
 * @param self The object
 * @return A static buffer with the representation.
 * Note: the buffer is only 1024 characters long.
 *     	Use Dy_Repr() and DyString_*() directly if that isn't enough.
 */
const char *Dy_AsRepr(DyObject *self);

// Length
/**
 * @brief Get an object's length
 * @arg self The object
 * @return The length
 * For lists: the number of items contained
 * For strings: the string length
 */
size_t      Dy_Length(DyObject *self);


// ----------------------------------------------------------------------------
// Constant Objects
extern DyObject *Dy_Undefined;
extern DyObject *Dy_None;
extern DyObject *Dy_True;
extern DyObject *Dy_False;

inline bool DyUndefined_Check(DyObject *obj)
{ return obj == Dy_Undefined; }

inline bool DyNone_Check(DyObject *obj)
{ return obj == Dy_None; }

inline bool DyBool_Check(DyObject *obj)
{ return obj == Dy_True || obj == Dy_False; }


// ----------------------------------------------------------------------------
// Numbers
inline bool DyLong_Check(DyObject *obj)
{ return Dy_Type(obj) == DY_LONG; }

DyObject *DyLong_New(int64_t value);

int64_t DyLong_Get(DyObject *self);

inline bool DyFloat_Check(DyObject *obj)
{ return Dy_Type(obj) == DY_FLOAT; }

DyObject *DyFloat_New(double value);

double DyFloat_Get(DyObject *self);


// ----------------------------------------------------------------------------
// Dictionaries
inline bool DyDict_Check(DyObject *obj)
{ return Dy_Type(obj) == DY_DICT; }

DyObject *DyDict_New();
DyObject *DyDict_NewWithParent(DyObject *parent);

bool   	  DyDict_Clear(DyObject *self);


// ----------------------------------------------------------------------------
// Lists
inline bool DyList_Check(DyObject *obj)
{ return Dy_Type(obj) == DY_LIST; }

DyObject *DyList_New();
DyObject *DyList_NewEx(size_t preallocate);

// These don't steal the refs!
bool      DyList_Clear(DyObject *self);
bool   	  DyList_Insert(DyObject *self, ssize_t where, DyObject *value);
bool      DyList_Append(DyObject *self, DyObject *value);


// ----------------------------------------------------------------------------
// Strings
// strings are UTF-8 encoded.
/**
 * @brief Check if an object is a string
 * @param obj The object
 */
inline bool DyString_Check(DyObject *obj)
{ return Dy_Type(obj) == DY_STRING; }

/**
 * @brief Create a string object from a character array and size
 * @param data The character data
 * @param size The string size
 * @return New reference to a string object
 * @sa dy_string.h
 * @sa DyString_InternStringFromStringAndSize
 */
DyObject *DyString_FromStringAndSize(const char *data, size_t size);

/**
 * @brief Create a string object from a NTBS
 * @param cstr The null-terminated C string
 * @return New reference to a string object
 * @sa DyString_FromStringAndSize
 * @sa DyString_InternStringFromString
 */
inline DyObject *DyString_FromString(const char *cstr)
{ return DyString_FromStringAndSize(cstr, strlen(cstr)); }

/**
 * @brief Get a c-string copy of the string object
 * @param self The string object
 * @return a new NTBS buffer with the same contents as @param self
 * NOTE: buffer must be freed with free()
 */
char *DyString_GetString(DyObject *self);

/**
 * @brief Get the buffer behind a string object
 * @param self The string object
 * @return A readonly character buffer
 * @sa DyString_GetString
 * @sa DyString_Size
 */
const char *DyString_AsString(DyObject *self);


// ----------------------------------------------------------------------------
// Callables
// Callables should always return new references or NULL on error.

/**
 * @brief Check if an object is callable
 * @param obj The object
 */
bool DyCallable_Check(DyObject *obj);

/**
 * @brief Call a callable object with a list of arguments
 * @param self The callable object
 * @param arglist The argument list
 * @return The call result
 */
DyObject *Dy_Call(DyObject *self, DyObject *arglist);

/**
 * @brief Call a callable object without arguments
 * @param self The callable object
 * @return The call result
 */
DyObject *Dy_Call0(DyObject *self);

/**
 * @brief Call a callable object with a single argument
 * @param self The callable object
 * @param arg The argument
 * @return The call result
 */
DyObject *Dy_Call1(DyObject *self, DyObject *arg);

/**
 * @brief Call a callable object with two arguments
 * @param self The callable object
 * @param arg1 The first argument
 * @param arg2 The second argument
 * @return The call result
 */
DyObject *Dy_Call2(DyObject *self, DyObject *arg1, DyObject *arg2);

/**
 * @brief Call a callable object with three arguments
 * @param self The callable object
 * @param arg1 The first argument
 * @param arg2 The second argument
 * @param arg3 The third argument
 * @return The call result
 */
DyObject *Dy_Call3(DyObject *self, DyObject *arg1, DyObject *arg2, DyObject *arg3);

#ifdef __cplusplus
}
#endif
