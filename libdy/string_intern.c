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

#include "freelist_p.h"
#include "string_p.h"
#include "host_p.h"
#include "dy_string.h"
#include "dy_error.h"

#include <stdio.h>

// ----------------------------------------------------------------------------
// String Interning
#define DY_INTERN_TABLE_SIZE 256
#define DY_INTERN_BLOCK_SIZE 64

typedef struct si_bucket_t {
    struct _DyStringObject *item;
    struct si_bucket_t *next;
} si_bucket_t;

typedef struct si_block_t {
    struct si_block_t *next;
    struct freelist_t freelist; // The Buckets need to immediately follow the freelist. (see freelist_struct())
    struct si_bucket_t buckets[DY_INTERN_BLOCK_SIZE];
} si_block_t;

static struct {
    struct si_block_t *blocks;
    struct si_bucket_t table[DY_INTERN_TABLE_SIZE];
} DyIntern;

// Fast implementation for string objects
DyObject *DyString_Interned(DyObject *o)
{
    if (o->type != DY_STRING)
    {
        DyErr_SetArgumentTypeError("DyString_Interned", 0, "String", Dy_GetTypeName(o->type));
        return NULL;
    }
    if (((DyStringObject *)o)->flags & DYSTRING_INTERNED)
        return o;

    DyStringObject *str = ((DyStringObject *)o);
    Dy_hash_t hash = string_hash(str);
    si_bucket_t *bucket = &DyIntern.table[hash % DY_INTERN_TABLE_SIZE];

    if (!bucket->item)
        return NULL;

    // Find in chain
    do if (str->size == bucket->item->size
           && hash == bucket->item->hash
           && !memcmp(str->data, bucket->item->data, str->size))
        return (DyObject*)bucket->item;
    while ((bucket = bucket->next));

    return NULL;
}

// Fast implementation for c strings (NTBS)
DyObject *DyString_InternedString(const char *s)
{
    size_t size = strlen(s);
    Dy_hash_t hash = DyHost.string_hash_fn(s, size);
    si_bucket_t *bucket = &DyIntern.table[hash % DY_INTERN_TABLE_SIZE];

    if (!bucket->item)
        return NULL;

    // Find in chain
    do if (size == bucket->item->size
           && hash == bucket->item->hash
           && !memcmp(s, bucket->item->data, size))
        return (DyObject*)bucket->item;
    while ((bucket = bucket->next));

    return NULL;
}

// Get an empty bucket
static inline si_bucket_t *si_get_free_bucket()
{
    for (si_block_t *block = DyIntern.blocks; block; block = block->next)
        if (!freelist_empty2(block))
    		return freelist_pop2(block);

    si_block_t *block = malloc(sizeof(si_block_t));
    freelist_init(&block->freelist, sizeof(si_bucket_t), DY_INTERN_BLOCK_SIZE);

    block->next = DyIntern.blocks;
    DyIntern.blocks = block;

    return freelist_pop2(block);
}

// Intern an existing string
DyObject *DyString_Intern(DyObject *o)
{
    if (o->type != DY_STRING)
    {
        DyErr_SetArgumentTypeError("DyString_Intern", 0, "String", Dy_GetTypeName(o->type));
        return NULL;
    }
    if (((DyStringObject *)o)->flags & DYSTRING_INTERNED)
        return o;

    DyStringObject *str = ((DyStringObject *)o);
    Dy_hash_t hash = string_hash(str);
    si_bucket_t *bucket = &DyIntern.table[hash % DY_INTERN_TABLE_SIZE];
    si_bucket_t *tbucket = bucket;

    if (!bucket->item)
    {
        // Insert into table
        bucket->item = str;
        str->flags |= DYSTRING_INTERNED;
        return o;
    }

    // Find in chain
    do if (str->size == bucket->item->size
           && hash == bucket->item->hash
           && !memcmp(str->data, bucket->item->data, str->size))
        return (DyObject*)bucket->item;
    while ((bucket = bucket->next));

    // Insert in chain
    bucket = si_get_free_bucket();
    bucket->item = str;
    
    bucket->next = tbucket->next; // XXX inverse
    tbucket->next = bucket;
    
    str->flags |= DYSTRING_INTERNED;
    
    return o;
}

void DyString_InternInplace(DyObject **strp);

// Create new intern string
DyObject *DyString_InternStringFromStringAndSize(const char *s, size_t size)
{
    Dy_hash_t hash = DyHost.string_hash_fn(s, size);
    si_bucket_t *bucket = &DyIntern.table[hash % DY_INTERN_TABLE_SIZE];
    si_bucket_t *tbucket = bucket;

    if (!bucket->item)
    {
        // Insert into table
        DyStringObject *str = string_new(s, size);
        str->hash = hash;
        bucket->item = str;
        str->flags |= DYSTRING_INTERNED;
        return (DyObject*)str;
    }

    // Find in chain
    do if (size == bucket->item->size
           && hash == bucket->item->hash
           && !memcmp(s, bucket->item->data, size))
        return Dy_Retain((DyObject*)bucket->item);
    while ((bucket = bucket->next));

    // Insert in chain
    DyStringObject *str = string_new(s, size);
    str->hash = hash;
    str->flags |= DYSTRING_INTERNED;
    
    bucket = si_get_free_bucket();
    bucket->item = str;
    
    bucket->next = tbucket->next; // XXX inverse
    tbucket->next = bucket;

    return (DyObject*)str;
}

DyObject *DyString_InternStringFromString(const char *cstr);

inline static void si_free_bucket(si_bucket_t *bucket)
{
    // Find block the bucket belongs to
    for (si_block_t *block = DyIntern.blocks; block; block = block->next)
    	// Check if it lies in the memory region
    	if (block->buckets <= bucket && bucket < (block->buckets + DY_INTERN_BLOCK_SIZE))
    	{
    		// Free the memory
    		freelist_push2(block, bucket);
    		break;
    	}
}

void string_unintern(DyStringObject *o) // See also dict.c:find_and_remove_bucket()
{
    DyStringObject *str = ((DyStringObject *)o);
    Dy_hash_t hash = string_hash(str);
    si_bucket_t *bucket = &DyIntern.table[hash % DY_INTERN_TABLE_SIZE];
    si_bucket_t *tbucket = bucket;
    si_bucket_t *prev;

    // Find in chain
    //do if (str == bucket->item)
    do if (DyString_Equals(str, bucket->item))
    {
    	// If it's in the table, we can't free it
        if (bucket == tbucket)
    	{
    		// We might need to pull in the next one
            if (tbucket->next)
    		{
    			bucket = tbucket->next;
    			memcpy(tbucket, bucket, sizeof(si_bucket_t)); 	// Copy the bucket into the table
    			si_free_bucket(bucket);						// Free the now obsolete bucket
    		}
    		// Otherwise, we can simply invalidate it.
    		else
    			tbucket->item = NULL;
    	}
        else
        {
    		// Pull bucket out
            prev->next = bucket->next;
    		
    		// Free the bucket
    		si_free_bucket(bucket);
        }
    	
    	// We're done
    	break;
    }
    while ((bucket = (prev = bucket)->next));
}
