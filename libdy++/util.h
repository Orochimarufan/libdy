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
 * @file util.h
 * @brief Various utilities to be used in libdy++
 */

#pragma once

#include <libdy/types.h>


namespace Dy {

/// @addtogroup Exceptions
/// @{

/// @brief Exception ID for C++ conversion errors
#define LIBDY_ERROR_CXX_TYPE_ERROR "dy.TypeError.CXXConversionError"

/**
 * @brief Throw the active libdy exception as libdy++ Exception
 */
[[ noreturn ]] void throw_exception();

/**
 * @fn void throw_exception(DyObject *exception)
 * @brief Throw a libdy exception as libdy++ Exception
 * @param exception The exception
 * @note This will also set the libdy exception state if it isn't set to exception already.
 *       Doing so ensures consistency between libdy and libdy++
 */
[[ noreturn ]] void throw_exception(DyObject *exception);

/**
 * @fn void throw_exception(const char *errid, const char *message)
 * @brief Throw an exception in libdy and libdy++
 * @param errid The exception ID
 * @param message The error message
 * @sa format_exception(const char*, const char*)
 */
[[ noreturn ]] void throw_exception(const char *errid, const char *message);

/**
 * @fn void format_exception(const char *errid, const char *format, ...)
 * @brief Throw an exception in libdy and libdy++, allowing the message to be formatted
 * @param errid The exception ID
 * @param format The error message (printf-style) format
 * @sa throw_exception(const char*, const char*)
 */
[[ noreturn ]] void format_exception(const char *errid, const char *format, ...);

/// @}


namespace conv {

/**
 * @class convert
 * @ingroup Conversion
 * @brief A structure holding the conversion functions for T
 *
 * Currently, a specialization of this template can implement two members:
 *
 * @par from_value
 * @code static DyObject *from_value(T); @endcode
 * to create a libdy object from T. @n
 * from_value should return a <em>new reference</em> or <em>NULL and set a
 * a libdy (not libdy++) exception</em>
 *
 * @par to_value
 * @code static T to_value(DyObject *); @endcode
 * to create a native T object from a libdy object. @n
 * to_value should <em>not steal the reference</em> and <em>throw a
 * @link Exceptions libdy++ exception @endlink</em>.
 */
template <typename T>
struct convert {
};

/**
 * @brief Utility function for creating a DyObject * from a value
 * @ingroup Conversion
 * @param val The value
 * @return A DyObject * or NULL + exception on failure
 */
template <typename T>
inline DyObject *from_value(const T &val)
{
    return convert<T>::from_value(val);
}

/**
 * @brief Utility function for converting a libdy Object into native ones
 * @ingroup Conversion
 * @param obj The object
 * @return A native value
 * @throws Exception if it cannot convert the object
 */
template <typename T>
inline T to_value(DyObject *obj)
{
    return convert<T>::to_value(obj);
}

}

namespace util {

/**
 * @class safe_ptr
 * @ingroup Utility
 * @brief Very simple smart pointer, free'ing resources using a C free function
 * Similar to STL unique_ptr, less powerful & less overhead
 * @tparam T The type to point to
 * @tparam freefn A function to free objects of type T
 */
template <typename T, void (*freefn)(T*)>
class safe_ptr
{
    T *ptr;

public:
    /// @brief The type of the value pointed to
    using value_type = T;

    /// @brief The type of a raw pointer to the value
    using ptr_type = T *;

    /**
     * @brief Create a %safe_ptr
     * @param ptr The raw pointer
     */
    inline safe_ptr(ptr_type ptr) :
        ptr(ptr)
    {}

    inline ~safe_ptr()
    {
        if (ptr)
            freefn(ptr);
    }

    /**
     * @brief Access members on the pointed-to structure
     */
    inline ptr_type operator->()
    {
        return ptr;
    }

    /**
     * @brief Dereference the pointer
     */
    inline value_type operator*()
    {
        return *ptr;
    }

    /**
     * @brief Implicit conversion to a raw pointer
     */
    inline operator ptr_type ()
    {
        return ptr;
    }

    /**
     * @brief invalidate the safe_ptr
     * MUST be called AFTER manually deleting the object
     */
    inline void invalidate()
    {
        ptr = nullptr;
    }

    /**
     * @brief pass off control over the safe_ptr
     * @return The raw pointer
     * This will extract the raw pointer and leave the safe_ptr invalidated.
     * Especially useful for returning objects
     */
    inline ptr_type pass()
    {
        ptr_type r = ptr;
        invalidate();
        return r;
    }

    /**
     * @brief Check if the target is NULL
     */
    inline bool operator!()
    {
        return !ptr;
    }

    /**
     * @brief Check if the pointer is valid
     */
    inline explicit operator bool()
    {
        return ptr;
    }
};

}
}
