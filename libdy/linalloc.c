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

#include "linalloc.h"


struct dy_linalloc_t {
    size_t freect;
    void *next;
    uint8_t data[0];
};


dy_linalloc_t *dy_linalloc_new(size_t size, void*(*malloc)(size_t))
{
    dy_linalloc_t *la = malloc(sizeof(struct dy_linalloc_t) + size);
    if (!la)
        return NULL;

    la->freect = size;
    la->next = la->data;
    return la;
}

void *dy_linalloc_malloc(dy_linalloc_t *la, size_t size)
{
    if (la->freect < size)
        return NULL;
    void *mem = la->next;
    la->next += size;
    return mem;
}

size_t dy_linalloc_remaining(dy_linalloc_t *la)
{
    return la->freect;
}
