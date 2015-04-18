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

/**
 * @file libdy/numbers.h
 * @brief libdy numerical objects (int, float)
 */
 
 #include "types.h"
 #include "config.h"
 
 #include <stdbool.h>
 #include <stdint.h>
 

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
///@{
///@name Integers
LIBDY_API bool DyLong_Check(DyObject *obj);

LIBDY_API DyObject *DyLong_New(int64_t value);

LIBDY_API int64_t DyLong_Get(DyObject *self);

///@}
// ----------------------------------------------------------------------------
///@{
///@name Floats
LIBDY_API bool DyFloat_Check(DyObject *obj);

LIBDY_API DyObject *DyFloat_New(double value);

LIBDY_API double DyFloat_Get(DyObject *self);

///@}
#ifdef __cplusplus
}
#endif
