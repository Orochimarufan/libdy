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

#include "dict_p.h"
#include "dy_p.h"
#include "dy_error.h"
#include "dy_string.h"
#include "host_p.h"
#include "string_p.h"

#include <stdio.h>


// Inlines
bool DyDict_Check(DyObject *);

// Prototypes
static bucket_t *find_bucket(DyDictObject *, DyObject *key, Dy_hash_t hash);
static bucket_t *find_or_create_bucket(DyDictObject *, DyObject *key, Dy_hash_t hash);

// Implementation
static inline void dict_init(DyDictObject *o)
{
}

DyObject *DyDict_New()
{
    DyDictObject *self = NEW(DyDictObject);
    Dy_InitObject((DyObject *)self, DY_DICT);
    dict_init(self);
    return (DyObject *)self;
}

DyObject *DyDict_NewWithParent(DyObject *parent)
{
    if (!DyDict_Check(parent))
    {
        DyErr_SetArgumentTypeError("DyDict_NewWithParent", 1, "Object", Dy_GetTypeName(parent->type));
        return NULL;
    }
    DyDictObject *self = NEW(DyDictObject);
    Dy_InitObject((DyObject *)self, DY_DICT);
    dict_init(self);
    self->parent = (DyDictObject*)Dy_Retain(parent);
    return (DyObject *)self;
}

bool dict_clean(DyDictObject *self)
{
    // Release items
    for (int i = 0; i < DY_TABLE_SIZE; ++i)
    {
        for (bucket_t *b = &self->table[i]; b; b = b->next)
        {
            if (b->key)
            {
                Dy_Release(b->key);
                Dy_Release(b->value);
            }
        }
    	self->table[i].key = NULL;
    	self->table[i].next = NULL;
    }

    // Free blocks
    bucket_block_t *block = self->blocks, *last = NULL;

    while(block)
    {
        block = (last = block)->next;
        free(last);
    }
    
    return true;
}

bool DyDict_Clear(DyObject *self)
{
    if (!DyDict_Check(self))
    {
        DyErr_SetArgumentTypeError("DyDict_Clear", 0, "Dict", Dy_GetTypeName(Dy_Type(self)));
        return false;
    }

    return dict_clean((DyDictObject*)self);
}

void dict_destroy(DyDictObject *o)
{
    // Clean refs
    dict_clean(o);

    if (o->parent)
        Dy_Release((DyObject*)o->parent);
}

static bucket_t *find_bucket(DyDictObject *o, DyObject *key, Dy_hash_t hash)
{
    bucket_t *bucket = &o->table[hash % DY_TABLE_SIZE];

    if (!bucket->key)
        return NULL;

    // Find in chain
    do if (/*hash == bucket->hash &&*/ Dy_Equals(key, bucket->key))
            return bucket;
    while ((bucket = bucket->next));

    return NULL;
}

static bucket_block_t *create_block(size_t bucket_count)
{
    bucket_block_t *block = malloc(sizeof(bucket_block_t) + sizeof(bucket_t) * bucket_count);

    block->size = bucket_count;
    block->next = NULL;
    
    freelist_init(&block->freelist, sizeof(bucket_t), bucket_count);
    
    return block;
}

static bucket_t *find_or_create_bucket(DyDictObject *o, DyObject *key, Dy_hash_t hash)
{
    bucket_t *first = &o->table[hash % DY_TABLE_SIZE];
    bucket_t *bucket = first;

    if (!bucket->key)
        return bucket;

    // Find in chain
    do if (hash == bucket->hash && Dy_Equals(key, bucket->key))
            return bucket;
    while ((bucket = bucket->next));

    // Create in chain
    if (!o->blocks || freelist_empty2(o->blocks))
    {
    	size_t block_size = o->blocks ? o->blocks->size * 2 : DY_BLOCK_SIZE;

        // Create new block
        bucket_block_t *block = create_block(block_size);

        // Add block to list
        block->next = o->blocks;
        o->blocks = block;
    }

    // Grab a bucket from the last block
    bucket = freelist_pop2(o->blocks);

    // Add it to the chain for the table slot
    bucket->next = first->next;
    first->next = bucket;

    return bucket;
}

inline static void free_bucket(DyDictObject *o, bucket_t *bucket)
{
    // Find block the bucket belongs to
    for (bucket_block_t *block = o->blocks; block; block = block->next)
    	// Check if it lies in the memory region
    	if (block->buckets <= bucket && bucket < (block->buckets + block->size))
    	{
    		//printf("Free bucket at %p from block %p\n", bucket, block);
    		// Free the memory
    		freelist_push2(block, bucket);
    		break;
    	}
}

static void find_and_remove_bucket(DyDictObject *o, DyObject *key, Dy_hash_t hash)
{
    bucket_t *bucket = &o->table[hash % DY_TABLE_SIZE];
    bucket_t *tbucket = bucket;
    bucket_t *prev;

    // Find in chain
    do if (Dy_Equals(key, bucket->key))
    {
    	// Keep references to release the objects
    	DyObject *key = bucket->key;
    	DyObject *val = bucket->value;

    	// If it's in the table, we can't free it
        if (bucket == tbucket)
    	{
    		// We might need to pull in the next one
            if (tbucket->next)
    		{
    			bucket = tbucket->next;
    			memcpy(tbucket, bucket, sizeof(bucket_t)); 	// Copy the bucket into the table
    			free_bucket(o, bucket);						// Free the now obsolete bucket
    		}
    		// Otherwise, we can simply invalidate it.
    		else
    			tbucket->key = NULL;
    	}
        else
        {
    		// Pull bucket out
            prev->next = bucket->next;
    		
    		// Free the bucket
    		free_bucket(o, bucket);
        }
    	
    	// Release Objects
    	Dy_Release(key);
    	Dy_Release(val);
    	
    	// We're done
    	break;
    }
    while ((bucket = (prev = bucket)->next));
}

static inline void TE__unhashable(DyObject *o)
{
    DyErr_Set(DY_ERRID_NOT_HASHABLE, "Cannot use unhashable type as object key.");
}

static inline void __KeyError(DyObject *key)
{
    DyObject *kr = Dy_Repr(key);
    DyErr_Set(DY_ERRID_KEY_ERROR, DyString_AsString(kr));
    Dy_Release(kr);
}

bool dict_setitem(DyDictObject *o, DyObject *key, DyObject *value)
{
    bucket_t *b;
    Dy_hash_t hash;

    if (!Dy_HashEx(key, &hash))
    {
        TE__unhashable(key);
        return false;
    }
    
    // Delete
    if (!value)
    {
    	find_and_remove_bucket(o, key, hash);
    	
    	return true;
    }

    // Set
    else
    {
    	b = find_or_create_bucket(o, key, hash);
    	b->hash = hash;

    	/*DyObject *s = Dy_Str(key);
    	printf("Insert Key: %s\n", DyString_AsString(s));
    	Dy_Release(s);*/

    	Dy_Retain(key);
    	Dy_Retain(value);
    	
    	if (b->key)
    		Dy_Release(b->key);
    	if (b->value)
    		Dy_Release(b->value);

    	b->key = key;
    	b->value = value;
    	
    	return true;
    }
}

bool dict_contains(DyDictObject *o, DyObject *key)
{
    Dy_hash_t hash;
    return Dy_HashEx(key, &hash) && (find_bucket(o, key, hash) != NULL || (o->parent && dict_contains(o->parent, key)));
}

// Get key
DyObject *dict_get(DyDictObject *self, DyObject *key, Dy_hash_t hash)
{
    bucket_t *b;

    b = find_bucket(self, key, hash);
    if (b)
        return b->value;
    else
        return Dy_Undefined;
}

DyObject *dict_getitemu(DyDictObject *self, DyObject *key)
{
    Dy_hash_t hash;

    if (!Dy_HashEx(key, &hash))
    {
        TE__unhashable(key);
        return NULL;
    }
    
    DyDictObject *cur = self;
    DyObject *result = Dy_Undefined;
    
    while (cur && result == Dy_Undefined)
    {
    	result = dict_get(cur, key, hash);
    	cur = cur->parent;
    }
    
    return result;
}

DyObject *dict_getitem(DyDictObject *self, DyObject *key)
{
    DyObject *result = dict_getitemu(self, key);
    if (result == Dy_Undefined)
    {
    	__KeyError(key);
    	return NULL;
    }
    else
    	return result;
}

#define DICT_REPR_BUFFER 8192

DyObject *dict_repr(DyDictObject *self)
{
    char buffer[DICT_REPR_BUFFER] = {'{', '\0'};
    int pos = 1;
    for (int i = 0; i < DY_TABLE_SIZE; ++i)
        for (bucket_t *b = &self->table[i]; b; b = b->next)
        {
            if (!b->key)
                continue;
            DyObject *k = Dy_Repr(b->key);
            DyObject *v = Dy_Repr(b->value);
            pos += snprintf(buffer + pos, DICT_REPR_BUFFER - pos, "%s: %s, ", DyString_AsString(k), DyString_AsString(v));
            Dy_Release(k);
            Dy_Release(v);
        }
    if (pos != 1)
        pos -= 2;
    memcpy(buffer + pos++, "}", 2);
    return DyString_FromStringAndSize(buffer, pos);
}
