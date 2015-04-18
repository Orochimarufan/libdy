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
 * @file libdy/exceptions_c.h
 * @brief Helper macros for working with libdy exceptions from C
 */

#pragma once

#include "exceptions.h"
#include <assert.h>

/// Ensure a exception is set when returning an error
#define dy_return_error(T) { \
    assert(DyErr_Occurred() && "return_error: error return without exception set."); \
    return T; \
}

#define dy_return_null dy_return_error(NULL)

/// Exception propagation macro
#define dy_propagate_error(retval) \
    { if (DyErr_Occurred()) return retval; }

#define dy_propagate_null() \
    dy_propagate_error(NULL)


/** @{
 * @name Error handler macros
 *
 * Usage:
 * @code
 * fn_call()
 * DY_ERR_HANDLER
 *     DY_ERR_CATCH("libdy.SomeError", e)
 *         do_something(e);
 *     DY_ERR_CATCH_ALL(e)
 *     {
 *         print_error(e);
 *         DY_ERR_RETHROW();
 *     }
 * DY_ERR_HANDLER_END
 * @endcode
 *
 * @warning do NOT escape a DY_ERR_HANDLER block using flow control!
 * You can use "DY_ERR_RETURN(x);" instead of "return x;" or
 * "DY_ERR_RETHROW_RETURN(e);" instead of "DY_ERR_RETHROW(); return e;"
 * for others, you can prefix them with DY_ERR_ESCAPE: "DY_ERR_ESCAPE break;"
 */

/// Begin a libdy error handler
#define DY_ERR_HANDLER \
    { \
        DyObject *__exception = DyErr_Occurred(); \
        if (__exception) \
        { \
            bool __ehandled = false, __edone = false; \
            {

/// Begin a catch clause
/// @sa DyErr_Filter for information on errid
#define DY_ERR_CATCH(errid, var) \
            } \
            if (!__edone) { \
                DyObject *var = __exception; \
                __ehandled = __edone = DyErr_Filter(var, errid); \
                if (__edone)

/// Begin a catch-all clause
#define DY_ERR_CATCH_ALL(var) \
            } \
            if (!__edone) { \
                DyObject *var = __exception; \
                __ehandled = __edone = true;

/// Allows to escape the error handler
#define DY_ERR_ESCAPE \
                if (__ehandled) \
                    DyErr_Clear();

/// Return from the function executing the error handler
/// Same as \code DY_ERR_ESCAPE return val; \endcode
#define DY_ERR_RETURN(val) \
                DY_ERR_ESCAPE return val

/// Rethrow the exception when the handler is left
#define DY_ERR_RETHROW() \
                    __ehandled = false;

/// Rethrow the exception and return from the executing function
#define DY_ERR_RETHROW_RETURN(val) \
                    return val;

/// Leave a libdy error handler
#define DY_ERR_HANDLER_END \
            } \
            DY_ERR_ESCAPE \
        } \
    }

///@}