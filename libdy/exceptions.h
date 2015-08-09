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
 * @file libdy/exceptions.h
 * @brief libdy Exceptions
 */

#pragma once

#include "types.h"
#include "config.h"

#include <stdarg.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file libdy/exceptions.h
 * @brief Error handling code
 * @note error state is stored per-thread.
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
LIBDY_API DyObject *  DyErr_Occurred();

/**
 * @brief Clear the current error state
 * @note this will release the global reference to the Exception object.
 * @attention Make sure to take a reference to the borrowed return value of
 * DyErr_Occurred() before calling this if you intend to keep the Exception object.
 */
LIBDY_API void        DyErr_Clear();

/**
 * @brief Set the thread error state
 * @param errid The error identifier. Should be qualified like "libdy.IndexError"
 * @param message A human-readable message
 * @return An Exception object
 * @sa DyErr_Format
 * @sa DyErr_SetEx
 */
LIBDY_API DyObject *  DyErr_Set(const char *errid, const char *message);

/**
 * @brief Set the thread error state, constructing the message
 * @param errid The error id
 * @param format The printf format
 * @return A pointer to the exception object
 * @sa DyErr_Set
 */
LIBDY_API DyObject *    DyErr_Format(const char *errid, const char *format, ...);

/**
 * @brief Set the thread error state, constructing the message (va_list version)
 * @param errid The error id
 * @param format The printf format
 * @param args The vprintf va_list
 * @return A pointer to the exception object
 * @sa DyErr_Set
 */
LIBDY_API DyObject *    DyErr_FormatV(const char *errid, const char *format, va_list args);

/**
 * @brief Set the error state to an Exception object.
 * @param exception The Exception object
 * @return exception
 * @sa DyErr_DiscardAndSetObject
 * @note The exception's cause will be set by this function
 */
LIBDY_API DyObject *  DyErr_SetObject(DyObject *exception);

/**
 * @brief Discard the current error state and set it to a new Exception object.
 * @param exception The Exception object
 * @return exception
 * @sa DyErr_SetObject
 * @note Compared to DyErr_SetObject, the cause will be left untouched.
 */
LIBDY_API DyObject *  DyErr_DiscardAndSetObject(DyObject *exception);

/**
 * @brief Add data to an Exception object
 * @param exception The exception object
 * @param data The data
 * @param destructor The destructor function (may be NULL)
 * @return false with exception set on error
 * @warning Only use once, otherwise the first destructor never gets called.
 */
LIBDY_API bool DyErr_SetExceptionData(DyObject *exception, void *data, DyDataDestructor destructor);

// Argument checking helpers
/**
 * @brief Throw an argument exception
 * @param fname The function name
 * @param arg The argument number
 * @param expected The expected type name
 * @param got The received type name
 * @return An Exception object
 */
LIBDY_API DyObject *  DyErr_SetArgumentTypeError(const char *fname, int arg, const char *expected, const char *got);

/**
 * @brief Do argument checking
 * @param fname The function name (for the error message)
 * @param arg The argument number (for the error message)
 * @param expected The expected type
 * @param got The received argument
 * @return NULL on success | an Exception object on failure
 * @sa DyErr_SetArgumentTypeError
 *
 * Throws an ArgumentTypeError if the type of \c got is not \c expected.
 */
LIBDY_API DyObject *  DyErr_CheckArg(const char *fname, int arg, DyObjectType expected, DyObject *got);

// Memory Error
/**
 * @brief Throw a Out of Memory Exception
 * @return The exception object
 * @attention Use this to throw MemoryError as it uses a static Exception object.
 * This prevents running out of memory while creating the Exception.
 */
LIBDY_API DyObject *    DyErr_SetMemoryError();

// Work with Exception objects
/**
 * @brief Check if the exception is applicable
 * @param exception The exception object
 * @param errid The errid to check
 * @return whether the exception is applicable. false if \c exception was NULL or isn't an Exception object
 * @note "applicable" means that either the errids match, or the exception errid starts with "<errid>."
 * Which means: "dy.TypeError" matches "dy.TypeError" as well as "dy.TypeError.ArgumentError"
 */
LIBDY_API bool        DyErr_Filter(DyObject *exception, const char *errid);

/**
 * @brief Retrieve the error id
 * @param self The Exception object
 * @return The error id
 * @note The returned buffer is owned by the Exception object.
 */
LIBDY_API const char *DyErr_ErrId(DyObject *self);

/**
 * @brief Retrieve the error message
 * @param self The Exception object
 * @return A human-readable message describing the exception
 */
LIBDY_API const char *DyErr_Message(DyObject *self);

/**
 * @brief Retrieve an exception's cause
 * @param self The Exception object
 * @return A borrowed reference to the cause or NULL if there is none
 *
 * The Exception cause is any other exception that was active while this one was thrown.
 */
LIBDY_API DyObject *  DyErr_Cause(DyObject *self);

/**
 * @brief Retrieve any custom data added to the exception
 * @param self The Exception object
 */
LIBDY_API void *      DyErr_Data(DyObject *self);

#ifdef __cplusplus
}
#endif
