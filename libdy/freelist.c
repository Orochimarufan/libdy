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

#include <assert.h>

// Inlines
freelist_entry_t *freelist_pop(freelist_t *fl);
void freelist_push(freelist_t *fl, freelist_entry_t *entry);
bool freelist_empty(freelist_t *fl);

size_t freelist_len(freelist_t *fl)
{
    size_t size = 0;

    for (freelist_entry_t *e = fl->root; e; e = e->next)
    	size++;

    return size;
}

void freelist_init(freelist_t *fl, size_t item_size, size_t item_count)
{
    assert (item_size >= sizeof(freelist_entry_t) && "item_size must be >= sizeof(freelist_entry_t)");

    freelist_entry_t *prev = (freelist_entry_t*)&fl->root;
    freelist_entry_t *end = (freelist_entry_t*)(fl->data + item_size * item_count);
    
    // Caution: Cannot use [curr += item_size] because that would result in [curr = (void*)curr + item_size * sizeof(*curr)]
    // Which we don't want. Our slots are NOT aligned agains freelist_entry_t.
    for (freelist_entry_t *curr = (freelist_entry_t*)fl->data; curr < end; curr = (void*)curr + item_size)
    {
    	prev->next = curr;
    	prev = curr;
    }
    prev->next = NULL;
}

freelist_t *freelist_new(size_t item_size, size_t item_count, void*(*malloc)(size_t))
{
    freelist_t *fl = malloc(sizeof(freelist_t) + item_size * item_count);
    
    freelist_init(fl, item_size, item_count);
    
    return fl;
}
