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

#include "host_p.h"

#include <assert.h>

struct _DyHost DyHost = {
    .string_hash_fn = &Dy_hash_fnv1,
    .dict_table_size = 16,
    .dict_block_size = 16,
    .mm = {
        .malloc = malloc,
        .free = free,
        .realloc = realloc,
    },
};

void DyHost_SetHashFunc(Dy_string_hash_fn func)
{
    DyHost.string_hash_fn = func;
}

void DyHost_SetMemoryManager(Dy_MemoryManager_t mm)
{
    assert(mm.malloc);
    assert(mm.free);
    assert(mm.realloc);
    DyHost.mm = mm;
}
