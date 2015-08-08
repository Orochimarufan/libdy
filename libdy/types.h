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

/**
 * @file libdy/types.h
 * @brief Type definitions for common libdy types
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
/// Opaque DyObject struct
typedef struct _DyObject DyObject;

// ----------------------------------------------------------------------------
/// DyObject Types
typedef enum _DyObject_Type
{
    DY_INVALID = -1,
    DY_NONE = 0,
    DY_BOOL,
    DY_LONG,
    DY_FLOAT,
    DY_STRING,
    DY_DICT,
    DY_LIST,
    DY_USERDATA,
    DY_EXCEPTION,
} DyObject_Type;

// ----------------------------------------------------------------------------
/// Hash type
typedef long Dy_hash_t;

#ifdef __cplusplus
}
#endif
