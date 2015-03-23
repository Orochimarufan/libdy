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

#include "dy_buildstring.h"
#include "dy.h"
#include "host_p.h"
#include "string_p.h"

#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>


dy_buildstring_t *dy_buildstring_new(const char *part, size_t size)
{
    dy_buildstring_t *bs = dy_malloc(sizeof(dy_buildstring_t));
    if (!bs)
        return NULL;

    bs->part = part;
    bs->part_size = size;
    bs->free_part = NULL;
    bs->next = NULL;

    return bs;
}

inline static void bs_add(dy_buildstring_t *bs, dy_buildstring_t *nbs)
{
    while (bs->next)
        bs = bs->next;
    bs->next = nbs;
}

dy_buildstring_t *dy_buildstring_append(dy_buildstring_t *bs, const char *part, size_t size)
{
    dy_buildstring_t *nbs = dy_buildstring_new(part, size);
    if (!nbs)
        return NULL;

    bs_add(bs, nbs);

    return nbs;
}

dy_buildstring_t *dy_buildstring_more(dy_buildstring_t **bs, const char *part, size_t size)
{
    if (!*bs)
        return *bs = dy_buildstring_new(part, size);
    else
        return dy_buildstring_append(*bs, part, size);
}

size_t dy_buildstring_size(dy_buildstring_t *bs)
{
    size_t size = 0;
    while (bs)
    {
        size += bs->part_size;
        bs = bs->next;
    }
    return size;
}

size_t dy_buildstring_copy(char *dest, dy_buildstring_t *bs, size_t space)
{
    size_t size = 0;
    const char *begin = dest;
    while (bs && space)
    {
        size_t chunk = smin(space, bs->part_size);
        memcpy(dest, bs->part, chunk);
        dest += chunk;
        space -= chunk;
        bs = bs->next;
    }
    return dest - begin;
}

DyObject *dy_buildstring_build(dy_buildstring_t *bs)
{
    DyStringObject *str = string_new_ex(dy_buildstring_size(bs));
    if (!str)
        return NULL;

    size_t size = dy_buildstring_copy(str->data, bs, str->size);
    str->data[size] = 0;

    return (DyObject *)str;
}

inline static void bs_free(dy_buildstring_t *bs, void(*fallback_free)(void*))
{
    if (bs->free_part)
        bs->free_part((char*)bs->part);
    else if (fallback_free)
        fallback_free((void*)bs->part);
    dy_free(bs);
}

inline static void bs_free_rec(dy_buildstring_t *bs, void (*fallback_free)(void*))
{
    dy_buildstring_t *last;
    while (bs)
    {
        last = bs;
        bs = bs->next;
        bs_free(last, fallback_free);
    }
}

void dy_buildstring_free(dy_buildstring_t *bs)
{
    bs_free_rec(bs, NULL);
}

void dy_buildstring_free_content(dy_buildstring_t *bs, void(*free)(void*))
{
    bs_free_rec(bs, free);
}

dy_buildstring_t *dy_buildstring_printf(dy_buildstring_t *bs, const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, va);
    va_end(va);

    char *buf = dy_malloc(size + 1);
    if (!buf)
        return NULL;

    dy_buildstring_t *nbs = dy_buildstring_new(buf, size);
    if (!nbs)
    {
        dy_free(buf);
        return NULL;
    }

    va_start(va, fmt);
    vsnprintf(buf, size + 1, fmt, va);
    va_end(va);

    nbs->free_part = (dy_buildstring_part_free_fn) dy_free;

    bs_add(bs, nbs);

    return nbs;
}


// =============================================================================
// DyString integration
// -----------------------------------------------------------------------------
#include "string_p.h"
#include "dy_error.h"

static void free_dystring(char *data)
{
    DyObject *str = (DyObject*)(data - offsetof(DyStringObject, data));
    Dy_Release(str);
}

dy_buildstring_t *dy_buildstring_append2(dy_buildstring_t *bs, DyObject *str)
{
    if (DyErr_CheckArg("dy_buildstring_append2", 1, DY_STRING, str))
        return_null;

    DyStringObject *so = (DyStringObject *)str;

    dy_buildstring_t *nbs = dy_buildstring_new(so->data, so->size);
    if (!nbs)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    Dy_Retain(str);
    nbs->free_part = free_dystring;

    bs_add(bs, nbs);

    return nbs;
}

dy_buildstring_t *dy_buildstring_repr(dy_buildstring_t *bs, DyObject *s)
{
    return bsrepr(bs, s);
}
