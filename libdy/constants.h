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
 * @file libdy/constants.h
 * @brief Constant objects
 */

#pragma once
 
#include "types.h"
#include "config.h"
 
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
// Constant Objects
/**
 * @brief Undefined
 * @warning This is a special value in that it marks things as non-existent.
 * Don't use it as a meaningful value since it already serves a special purpose
 * in some libdy APIs. Returning Undefined should in most cases be considered
 * an error.
 */
extern LIBDY_API DyObject *Dy_Undefined;

/**
 * @brief None
 *
 * libdy's equivalent to NULL in other languages.
 * It provides a way to explicitly specify nothingness.
 */
extern LIBDY_API DyObject *Dy_None;

/**
 * @brief True
 * @sa False
 * @note This is NOT equivalent to any numerical value!
 *
 * The positive truth value
 */
extern LIBDY_API DyObject *Dy_True;

/**
 * @brief False
 * @sa True
 * @note this is NOT equivalent to any numerical value!
 */
extern LIBDY_API DyObject *Dy_False;

LIBDY_API bool DyUndefined_Check(DyObject *obj);

LIBDY_API bool DyNone_Check(DyObject *obj);

LIBDY_API bool DyBool_Check(DyObject *obj);


#ifdef __cplusplus
}
#endif
