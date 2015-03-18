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
#include "dy_error.h"

#include <assert.h>

// Inlines
bool DyList_Check(DyObject *);

// Modeled after cpython/Objects/listobject.c
DyObject *DyList_New()
{
    DyListObject *self = NEW(DyListObject);
    Dy_InitObject((DyObject *)self, DY_LIST);
    return (DyObject *)self;
}

DyObject *DyList_NewEx(size_t allocate)
{
    DyListObject *self = NEW(DyListObject);
    Dy_InitObject((DyObject *)self, DY_LIST);
    self->allocated = allocate;
    self->items = malloc(sizeof(DyObject *) * allocate);
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
    
    new_allocated = (new_size >> 3) + (new_size < 9 ? 3 : 6);
    
    if (new_size == 0)
    	new_allocated = 0;
    
    items = realloc(self->items, sizeof(DyObject*) * new_allocated);
    
    if (items == NULL)
    {
    	DyErr_Set(DY_ERRID_MEMORY_ERROR, "Could not allocate memory for list object");
    	return -1;
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
    	return NULL;
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
    	return false;
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
    	return false;
    
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
    	return false;
    
    return list_insert((DyListObject *)self, where, value);
}

inline static bool list_append(DyListObject *self, DyObject *value)
{
    size_t n = self->size;

    if (list_resize(self, n + 1) == -1)
    	return false;
    
    self->items[n] = Dy_Retain(value);
    return true;
}

bool DyList_Append(DyObject *self, DyObject *value)
{
    if (DyErr_CheckArg("DyList_Append", 0, DY_LIST, self))
    	return false;
    
    return list_append((DyListObject *)self, value);
}

void list_destroy(DyListObject *self)
{
    if (self->items != NULL)
    {
    	for (ssize_t i = self->size; --i >= 0; )
    		Dy_Release(self->items[i]);
    	free(self->items);
    }
}

#define DY_LIST_REPR_BUFFER 8192

DyObject *list_repr(DyListObject *self)
{
    char buffer[DY_LIST_REPR_BUFFER + 5] = { '[', 0 };
    size_t pos = 1;
    
    for (size_t i = 0; i < self->size && pos < DY_LIST_REPR_BUFFER ; ++i)
    {
    	DyObject *repr = Dy_Repr(self->items[i]);
    	strncpy(buffer + pos, DyString_AsString(repr), DY_LIST_REPR_BUFFER - pos);
    	pos += Dy_Length(repr);
    	Dy_Release(repr);
        buffer[pos++] = ',';
        buffer[pos++] = ' ';
    }

    if (self->size > 0)
        pos -= 2;
    
    buffer[pos++] = ']';
    buffer[pos++] = 0;

    return DyString_FromStringAndSize(buffer, pos);
}

bool DyList_Clear(DyObject *self)
{
    if (DyErr_CheckArg("DyList_Clear", 0, DY_LIST, self))
    	return false;

    if (((DyListObject *)self)->items != NULL)
    {
    	for (ssize_t i = ((DyListObject *)self)->size; --i >= 0; )
    		Dy_Release(((DyListObject *)self)->items[i]);
    	free(((DyListObject *)self)->items);
    	((DyListObject *)self)->items = NULL;
    	((DyListObject *)self)->size = 0;
    }
    return true;
}
