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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "dy.h"

#include <stdlib.h>

/**
 * @file dy_host.h
 * @brief Managing the library
 * This should only be used by ONE party.
 * In plugin infrastructures, this will usually be the plugin host.
 */

typedef Dy_hash_t (*Dy_string_hash_fn)(const char *data, size_t length);

/**
 * @brief Specify the hash function used for strings
 * @param func The hash function
 * This must be done before using ANY libdy objects, as it modifies
 * behavior of string and mapping objects.
 */
void DyHost_SetHashFunc(Dy_string_hash_fn func);

// Predefined hash functions:
Dy_hash_t Dy_hash_fnv1(const char *data, size_t length);
Dy_hash_t Dy_hash_Murmur3_32(const char *data, size_t length);

#ifdef __cplusplus
}
#endif