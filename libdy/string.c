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

#include "string_p.h"
#include "host_p.h"
#include "dy_error.h"
#include "dy_string.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <strings.h>
#include <stdio.h>

// Inlines
bool DyString_Check(DyObject *obj);
DyObject *DyString_FromString(const char *cstr);

// Implementation
DyStringObject *string_new_ex(size_t size)
{
    DyStringObject *o = dy_malloc(sizeof(DyStringObject) + size);
    if (!o)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    Dy_InitObject((DyObject*)o, DY_STRING);

    o->flags = 0;
    o->size = size;

    return o;
}

DyStringObject *string_new(const char *s, size_t size)
{
    DyStringObject *o = string_new_ex(size);
    if (!o)
        return_null;

    memcpy(o->data, s, size);
    o->data[size] = 0;

    return o;
}

DyObject *DyString_FromStringAndSize(const char *data, size_t size)
{
    //if (size < 16)
    //    return DyString_InternStringFromStringAndSize(data, size);
    return (DyObject *)string_new(data, size);
}

void string_destroy(DyStringObject *o)
{
    if (o->flags & DYSTRING_INTERNED)
        string_unintern(o);
}

Dy_hash_t string_hash(DyStringObject *o)
{
    if (!o->flags & DYSTRING_HASH)
    {
        o->hash = DyHost.string_hash_fn(o->data, o->size);
        o->flags |= DYSTRING_HASH;
    }
    return o->hash;
}

inline bool DyString_Equals(DyStringObject *a, DyStringObject *b)
{
    return a->size == b->size && !memcmp(a->data, b->data, a->size);
}

/*char *DyString_GetString(DyObject *self)
{
    if (!DyString_Check(self))
    {
        DyErr_SetArgumentTypeError("DyString_GetString", 0, "String", Dy_GetTypeName(self->type));
        return NULL;
    }

    char *buf = malloc(((DyStringObject *)self)->size+1);
    memcpy(buf, ((DyStringObject *)self)->data, ((DyStringObject *)self)->size);
    buf[((DyStringObject *)self)->size] = 0;
    return buf;
}*/

const char *DyString_AsString(DyObject *self)
{
    if (DyErr_CheckArg("DyString_AsString", 0, DY_STRING, self))
    	return_null;

    return ((DyStringObject *)self)->data;
}

DyObject *string_repr(DyStringObject *self)
{
    char *x = dy_malloc(self->size + 3);
    snprintf(x, self->size + 3, "\"%s\"", self->data);
    DyObject *s = DyString_FromStringAndSize(x, self->size + 2);
    dy_free(x);
    return s;
}

// Repr ------------------------------------------------------------------------
#include "dy_buildstring.h"

dy_buildstring_t *string_bsrepr(dy_buildstring_t *bs, DyStringObject *self)
{
    dy_buildstring_t *lbs = dy_buildstring_append(bs, "\"", 1);
    if (!lbs)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    lbs = dy_buildstring_append2(lbs, (DyObject*)self);

    if (!lbs)
        return_null;

    lbs = dy_buildstring_append(lbs, "\"", 1);
    if (!lbs)
        DyErr_SetMemoryError();
    return lbs;
}
