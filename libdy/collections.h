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
 * @file collections.h
 * @brief Item collections (list, dict)
 *
 * libdy provides 2 types of collections: lists and dictionaries (mappings)
 */

#pragma once

#include "types.h"
#include "config.h"

#include <stdbool.h>
#include <sys/types.h> // ssize_t; TODO: find portable solution

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
///@{
///@name Lists
LIBDY_API bool DyList_Check(DyObject *obj);

/**
 * @brief Create a new list object
 * @return A new libdy list object
 * @sa DyList_NewEx
 */
LIBDY_API DyObject *DyList_New();

/**
 * @brief Create a new list object and pre-allocate some space
 * @param preallocate The number of items to pre-allocate for
 * @return A new libdy list object
 */
LIBDY_API DyObject *DyList_NewEx(size_t preallocate);

// These don't steal the refs!
/**
 * @brief Clear the contents of a list object
 * @param self The list object
 * @return Whether the operation succeeded
 */
LIBDY_API bool      DyList_Clear(DyObject *self);

/**
 * @brief Insert an item into the list at a given index
 * @param self The list object
 * @param where The index to insert the item at
 * @param value The item
 * @return Whether the operation succeeded
 */
LIBDY_API bool      DyList_Insert(DyObject *self, ssize_t where, DyObject *value);

/**
 * @brief Append an item to the list
 * @param self The list object
 * @param value The item
 * @return Whether the operation succeeded
 */
LIBDY_API bool      DyList_Append(DyObject *self, DyObject *value);

///@}
// ----------------------------------------------------------------------------
///@{
///@name Dictionaries
LIBDY_API bool      DyDict_Check(DyObject *obj);

/**
 * @brief Create a new libdy dictionary
 * @return A new libdy dicitionary object
 * @sa DyDict_NewWithParent
 */
LIBDY_API DyObject *DyDict_New();

/**
 * @brief Create a new libdy dictionary, dynamically inheriting items from a parent
 * @param parent The parent dictionary
 * @return A new libdy dictionary object
 */
LIBDY_API DyObject *DyDict_NewWithParent(DyObject *parent);

/**
 * @brief Clear all items from a dictionary
 * @param self The dictionary
 * @return Whether the operation succeeded
 */
LIBDY_API bool      DyDict_Clear(DyObject *self);

///@}
// ----------------------------------------------------------------------------
///@{
///@name Subscription
///@brief Item subscription interface
/**
 * @brief Retrieve an index from an object/list
 * @param self The object
 * @param key The key
 * @return A borrowed reference to the object stored in \c self for \c key
 * @throw [dy.KeyError] when \c key isn't in \c self
 */
LIBDY_API DyObject *Dy_GetItem(DyObject *self, DyObject *key);
LIBDY_API DyObject *Dy_GetItemString(DyObject *self, const char *key);
LIBDY_API DyObject *Dy_GetItemLong(DyObject *self, long key);

/**
 * @brief Retrieve an index from a dictionary/list
 * @param self The subscriptable object
 * @param key The key to look up
 * @param def A fallback value
 * @return A borrowed reference to the object || \c def
 */
LIBDY_API DyObject *Dy_GetItemD(DyObject *self, DyObject *key, DyObject *def);
LIBDY_API DyObject *Dy_GetItemStringD(DyObject *self, const char *key, DyObject *def);
LIBDY_API DyObject *Dy_GetItemLongD(DyObject *self, long key, DyObject *def);

/**
 * @brief Retrieve an index from a dictionary/list
 * @param self The subscriptable object
 * @param key The key to look up
 * @return A borrowed reference to the object || Dy_Undefined
 */
LIBDY_API DyObject *Dy_GetItemU(DyObject *self, DyObject *key);
LIBDY_API DyObject *Dy_GetItemStringU(DyObject *self, const char *key);
LIBDY_API DyObject *Dy_GetItemLongU(DyObject *self, long key);

/**
 * @brief Set an item in an object/list
 * @param self The object
 * @param key The key
 * @param value the new value
 * @return Whether the operation succeeded
 * @remark If \c value is NULL, \c key is deleted instead
 */
LIBDY_API bool      Dy_SetItem(DyObject *self, DyObject *key, DyObject *value);
LIBDY_API bool      Dy_SetItemString(DyObject *self, const char *key, DyObject *value);
LIBDY_API bool      Dy_SetItemLong(DyObject *self, long key, DyObject *value);

/**
 * @brief Check if an object/list contains a specific item
 * @param self The object
 * @param key The key
 * @return whether \c key exists in \c self
 * @attention Not Implemented
 */
LIBDY_API bool      Dy_Contains(DyObject *self, DyObject *key);
LIBDY_API bool      Dy_ContainsString(DyObject *self, const char *key);
LIBDY_API bool      Dy_ContainsLong(DyObject *self, long key);

///@}

#ifdef __cplusplus
}
#endif
