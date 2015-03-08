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

/**
 * @file error.h
 * @brief Error handling code
 * Note: error state is stored per-thread.
 */

// Error IDs
#define DY_ERRID_TYPE_ERROR     	"dy.TypeError"
#define DY_ERRID_ARGUMENT_TYPE     	"dy.TypeError.ArgumentError"
#define DY_ERRID_NOT_HASHABLE    	"dy.TypeError.UnhashableError"
#define DY_ERRID_CALL_ERROR    		"dy.CallError"
#define DY_ERRID_ARGUMENT_COUNT    	"dy.CallError.ArgumentCountError"
#define DY_ERRID_KEY_ERROR     		"dy.KeyError"
#define DY_ERRID_INDEX_ERROR     	"dy.KeyError.IndexError"
#define DY_ERRID_MEMORY_ERROR     	"dy.MemoryError"

// Global error state
/**
 * @brief Check if an error occurred
 * @return A borrowed reference to the Exception object or NULL if none is active
 */
DyObject *  DyErr_Occurred();

/**
 * @brief Clear the current error state
 * NOTE: this will release the global reference to the Exception object.
 * Make sure to take a reference to the borrowed return value of DyErr_Occurred() before calling this.
 */
void        DyErr_Clear();

/**
 * @brief Set the thread error state
 * @param errid The error identifier. Should be qualified like "libdy.IndexError"
 * @param message A human-readable message
 * @return An Exception object
 */
DyObject *  DyErr_Set(const char *errid, const char *message);

DyObject *    DyErr_SetEx(const char *errid, const char *message, void *data);

DyObject *    DyErr_Format(const char *errid, const char *format, ...);

/**
 * @brief Set the error state to an Exception object.
 * @param exception The Exception object
 * @return exception
 * @sa DyErr_DiscardAndSetObject
 * NOTE: The exception's cause will be set by this function
 */
DyObject *  DyErr_SetObject(DyObject *exception);

/**
 * @brief Discard the current error state and set it to a new Exception object.
 * @param exception The Exception object
 * @return exception
 * @sa DyErr_SetObject
 * NOTE: Compared to DyErr_SetObject, the cause will be left untouched.
 */
DyObject *  DyErr_DiscardAndSetObject(DyObject *exception);

// Argument checking helpers
/**
 * @brief Throw an argument exception
 * @param fname The function name
 * @param arg The argument number
 * @param expected The expected type name
 * @param got The received type name
 * @return An Exception object
 */
DyObject *  DyErr_SetArgumentTypeError(const char *fname, int arg, const char *expected, const char *got);

/**
 * @brief Do argument checking
 * @param fname The function name (for the error message)
 * @param arg The argument number (for the error message)
 * @param expected The expected type
 * @param got The received argument
 * @return NULL on success | an Exception object on failure
 * The thread error state will be set.
 */
DyObject *  DyErr_CheckArg(const char *fname, int arg, DyObject_Type expected, DyObject *got);

// Memory Error
/**
 * @brief Throw a Out of Memory Exception
 * @return The exception object
 * Use this instead of throwing a new one as it uses a static Exception object.
 * This prevents running out of memory while creating the Exception.
 */
DyObject *    DyErr_SetMemoryError();

// Work with Exception objects
/**
 * @brief Check if the exception is applicable
 * @param exception The exception object
 * @param errid The errid to check
 * @return whether the exception is applicable. false if \c exception was NULL or isn't an Exception object
 * "applicable" means that either the errids match, or the exception errid starts with "<errid>."
 * Which means: "dy.TypeError" matches "dy.TypeError" as well as "dy.TypeError.ArgumentError"
 */
bool        DyErr_Filter(DyObject *exception, const char *errid);

/**
 * @brief Retrieve the error id
 * @param self The Exception object
 * @return The error id
 * NOTE: The returned buffer is owned by the Exception object.
 */
const char *DyErr_ErrId(DyObject *self);

/**
 * @brief Retrieve the error message
 * @param self The Exception object
 * @return A human-readable message describing the exception
 */
const char *DyErr_Message(DyObject *self);

/**
 * @brief Retrieve an exception's cause
 * @param self The Exception object
 * @return A borrowed reference to the cause or NULL if there is none
 * The Exception cause is any other exception that was propagating while this one was thrown.
 */
DyObject *  DyErr_Cause(DyObject *self);

void *    	DyErr_Data(DyObject *self);

// Helper macros
#define DY_ERR_PROPAGATE(obj) \
    { if (Dy_CheckException(obj)) return obj; }

#define DY_ERR_PROPAGATE_GET() \
    { DyObject *e = DyErr_Occurred(); if (e) return e; }

#define DY_ERR_PROPAGATE_VAL(retval) \
    { if (DyErr_Occurred()) return retval; }

/*
 * Error handler macros
 * Usage:
 *
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
 *
 * NOTE: do NOT escape a DY_ERR_HANDLER block using flow control!
 * You can use "DY_ERR_RETURN(x);" instead of "return x;" or
 * "DY_ERR_RETHROW_RETURN(e);" instead of "DY_ERR_RETHROW(); return e;"
 * for others, you can prefix them with DY_ERR_ESCAPE: "DY_ERR_ESCAPE break;"
 */
#define DY_ERR_HANDLER \
    { \
        DyObject *__exception = DyErr_Occurred(); \
        if (__exception) \
        { \
            bool __ehandled = false, __edone = false; \
            {

#define DY_ERR_CATCH(errid, var) \
            } \
            if (!__edone) { \
                DyObject *var = __exception; \
                __ehandled = __edone = DyErr_Filter(var, errid); \
                if (__edone)

#define DY_ERR_CATCH_ALL(var) \
            } \
            if (!__edone) { \
                DyObject *var = __exception; \
                __ehandled = __edone = true;

#define DY_ERR_ESCAPE \
                if (__ehandled) \
                    DyErr_Clear();

#define DY_ERR_RETURN(val) \
                DY_ERR_ESCAPE \
                return val

#define DY_ERR_RETHROW() \
                    __ehandled = false;

#define DY_ERR_RETHROW_RETURN() \
                    return __exception;

#define DY_ERR_RETHROW_RETURN_VAL(val) \
                    return val;

#define DY_ERR_HANDLER_END \
            } \
            DY_ERR_ESCAPE \
        } \
    }


#ifdef __cplusplus
}
#endif
