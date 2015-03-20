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


dy_buildstring_t *dy_buildstring_new(const char *part, size_t size)
{
    dy_buildstring_t *bs = dy_malloc(sizeof(dy_buildstring_t));
    if (!bs)
        return NULL;

    bs->part = part;
    bs->part_size = size;
    bs->next = NULL;

    return bs;
}

dy_buildstring_t *dy_buildstring_append(dy_buildstring_t *bs, const char *part, size_t size)
{
    dy_buildstring_t *nbs = dy_buildstring_new(part, size);
    if (!nbs)
        return NULL;

    while (bs->next)
        bs = bs->next;
    bs->next = nbs;

    return nbs;
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

void dy_buildstring_free(dy_buildstring_t *bs)
{
    dy_buildstring_t *last;
    while (bs)
    {
        last = bs;
        bs = bs->next;
        dy_free(last);
    }
}

void dy_buildstring_free_content(dy_buildstring_t *bs, void(*free)(void*))
{
    dy_buildstring_t *last;
    while (bs)
    {
        last = bs;
        bs = bs->next;
        free((char*)last->part); // need to cast off const
        dy_free(last);
    }
}
