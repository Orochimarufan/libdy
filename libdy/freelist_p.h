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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @file freelist_p.h
 * @brief Simple freelist implementation
 * Note: This only supports allocating fixed-size elements from the freelist
 */

/**
 * @brief The internal freelist entry type.
 * Note: Items managed in the freelist cannot be smaller than this.
 */
typedef struct freelist_entry_t {
    struct freelist_entry_t *next;
} freelist_entry_t;

/**
 * @class freelist_t
 * @brief The generic type for the freelist
 * The data in the data buffer can be anything.
 * To the freelist, it's just a sequence of bytes.
 */
typedef struct freelist_t {
    freelist_entry_t *root;
    uint8_t data[0];
} freelist_t;

/**
 * @brief Allocate and Initialize a new freelist
 * @param item_size The size of one item
 * @param item_count The number of items
 * @return Pointer to a new freelist
 */
freelist_t *freelist_new(size_t item_size, size_t item_count);

/**
 * @brief Initialize a pre-allocated freelist
 * @param fl The freelist
 * @param item_size The size of one item
 * @param item_count The number of items
 * Attention: The user must make sure that fl has enough space.
 * The total space requirement is sizeof(freelist_t) + item_size * item_count
 */
void freelist_init(freelist_t *fl, size_t item_size, size_t item_count);

/**
 * @brief Add a free'd entry to the freelist
 * @param fl The freelist
 * @param entry The entry
 */
inline void freelist_push(freelist_t *fl, freelist_entry_t *entry)
{
    entry->next = fl->root;
    fl->root = entry;
}

/**
 * @brief Retrieve a currently-free entry from the freelist
 * @param fl The freelist
 * @return An entry
 * Attention: 
 */
inline freelist_entry_t *freelist_pop(freelist_t *fl)
{
    freelist_entry_t *entry = fl->root;
    fl->root = entry->next;

    return entry;
}

/**
 * @brief Check if the freelist is empty
 * @param fl The freelist
 * @return Whether or not it is empty
 */
inline bool freelist_empty(freelist_t *fl)
{
    return !fl->root;
}

/**
 * @brief Retrieve the length of the freelist (Number of free items)
 * @param fl The freelist
 * @return The number of items in the list
 */
size_t freelist_len(freelist_t *fl);


// Convenience Macros
#define freelist_newt(/* typename */type, /* size_t */count) \
    (freelist_new(sizeof(type), count))

#define freelist_struct(/* typename */type, /* name */fieldname) \
    struct { \
    	freelist_t freelist; \
    	type fieldname[0]; \
    }

#define freelist_pop2(/* freelist_struct */fl) \
    ((void*)freelist_pop(&fl->freelist))

#define freelist_push2(/* freelist_struct */fl, /* freelist_entry_t* */e) \
    (freelist_push(&fl->freelist, (freelist_entry_t*)e))

#define freelist_empty2(/* freelist_struct */fl) \
    (freelist_empty(&fl->freelist))
