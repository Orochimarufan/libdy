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

#include "list_p.h"
#include "exceptions.h"

#include <assert.h>

// Inlines
bool DyList_Check(DyObject *self)
{
    return self->type == DY_LIST;
}

// Modeled after cpython/Objects/listobject.c
DyObject *DyList_New()
{
    DyListObject *self = NEW(DyListObject);
    if (!self)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    Dy_InitObject((DyObject *)self, DY_LIST);

    return (DyObject *)self;
}

DyObject *DyList_NewEx(size_t allocate)
{
    DyListObject *self = NEW(DyListObject);
    if (!self)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    Dy_InitObject((DyObject *)self, DY_LIST);

    self->allocated = allocate;
    self->items = dy_malloc(sizeof(DyObject *) * allocate);

    return (DyObject *)self;
}

inline static int list_resize(DyListObject *self, size_t new_size)
{
    DyObject **items;
    size_t new_allocated;
    size_t allocated = self->allocated;
    
    if (allocated >= new_size && new_size >= (allocated >> 1))
    {
    	assert (self->items != NULL || new_size == 0);
    	self->size = new_size;
    	return 0;
    }
    
    new_allocated = new_size + (new_size >> 3) + (new_size < 9 ? 3 : 6);
    
    if (new_size == 0)
    	new_allocated = 0;
    
    items = dy_realloc(self->items, sizeof(DyObject*) * new_allocated);
    
    if (items == NULL)
    {
        DyErr_SetMemoryError();
    	return_error(-1);
    }
    
    self->items = items;
    self->size = new_size;
    self->allocated = new_allocated;
    return 0;
}

DyObject *list_getitem(DyListObject *self, ssize_t key)
{
    size_t size = self->size;
    
    if (key < 0)
    	key += size;
    
    if (key < 0 || key >= size)
    {
    	DyErr_Set(DY_ERRID_INDEX_ERROR, "List index out of range");
    	return_null;
    }
    
    return self->items[key];
}

DyObject *list_getitemu(DyListObject *self, ssize_t key)
{
    size_t size = self->size;
    
    if (key < 0)
    	key += size;
    
    if (key < 0 || key >= size)
    	return Dy_Undefined;
    
    return self->items[key];
}

bool list_setitem(DyListObject *self, ssize_t key, DyObject *value)
{
    size_t size = self->size;
    DyObject *olditem;
    DyObject **p;
    
    if (key < 0)
    	key += size;
    
    if (key < 0 || key >= size)
    {
    	DyErr_Set(DY_ERRID_INDEX_ERROR, "List assignment out of range");
    	return_error(false);
    }
    
    p = self->items + key;
    olditem = *p;
    *p = Dy_Retain(value);
    Dy_Release(olditem);
    
    return true;
}

inline static bool list_insert(DyListObject *self, ssize_t where, DyObject *value)
{
    ssize_t n = self->size;
    
    // Resize list
    if (list_resize(self, n + 1) == -1)
    	return_error(false);
    
    // Check bounds
    if (where < 0)
    {
    	where += n;
    	if (where < 0)
    		where = 0;
    }
    
    if (where > n)
    	where = n;
    
    // Insert
    DyObject **items = self->items;
    for (size_t i = (size_t)n; --i >= where; )
    	items[i+1] = items[i];
    Dy_Retain(value);
    items[where] = value;
    return true;
}

bool DyList_Insert(DyObject *self, ssize_t where, DyObject *value)
{
    if (DyErr_CheckArg("DyList_Insert", 0, DY_LIST, self))
    	return_error(false);
    
    return list_insert((DyListObject *)self, where, value);
}

inline static bool list_append(DyListObject *self, DyObject *value)
{
    size_t n = self->size;

    if (list_resize(self, n + 1) == -1)
    	return_error(false);
    
    self->items[n] = Dy_Retain(value);
    return true;
}

bool DyList_Append(DyObject *self, DyObject *value)
{
    if (DyErr_CheckArg("DyList_Append", 0, DY_LIST, self))
    	return_error(false);
    
    return list_append((DyListObject *)self, value);
}

void list_destroy(DyListObject *self)
{
    if (self->items != NULL)
    {
    	for (ssize_t i = self->size; --i >= 0; )
    		Dy_Release(self->items[i]);
    	dy_free(self->items);
    }
}

bool DyList_Clear(DyObject *self)
{
    if (DyErr_CheckArg("DyList_Clear", 0, DY_LIST, self))
    	return_error(false);

    if (((DyListObject *)self)->items != NULL)
    {
    	for (ssize_t i = ((DyListObject *)self)->size; --i >= 0; )
    		Dy_Release(((DyListObject *)self)->items[i]);
    	dy_free(((DyListObject *)self)->items);
    	((DyListObject *)self)->items = NULL;
    	((DyListObject *)self)->size = 0;
    }
    return true;
}


// Repr ------------------------------------------------------------------------
#include "buildstring.h"

dy_buildstring_t *list_bsrepr(dy_buildstring_t *bs, DyListObject *self)
{
    dy_buildstring_t *lbs = bs = dy_buildstring_append(bs, "[", 1);
    if (!lbs)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    for (size_t i = 0; i < (self->size < 20 ? self->size : 20); ++i)
    {
        lbs = bsrepr(lbs, self->items[i]);
        if (!lbs)
            return_null;

        lbs = dy_buildstring_append(lbs, ", ", 2);
        if (!lbs)
        {
            DyErr_SetMemoryError();
            return_null;
        }
    }

    if (lbs != bs)
    {
        lbs->part = "]";
        lbs->part_size = 1;
    }

    return lbs;
}

