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

#include <libdy/types.h>

#define LIBDY_ERROR_CXX_TYPE_ERROR "dy.TypeError.CXXConversionError"


namespace Dy {

[[ noreturn ]] void throw_exception();
[[ noreturn ]] void throw_exception(DyObject *exception);
[[ noreturn ]] void throw_exception(const char *errid, const char *message);
[[ noreturn ]] void format_exception(const char *errid, const char *format, ...);

namespace conv {

template <typename T>
struct convert {
    //static DyObject *from_value(T);
    //static T to_value(DyObject *);
};

template <typename T>
inline DyObject *from_value(const T &val)
{
    return convert<T>::from_value(val);
}

template <typename T>
inline T to_value(DyObject *obj)
{
    return convert<T>::to_value(obj);
}

}

namespace util {

/**
 * @class safe_ptr
 * @author Taeyeon Mori
 * @date 09/10/15
 * @file util.h
 * @brief Very simple smart pointer, free'ing resources using a C free function
 * Similar to STL unique_ptr, less powerful & less overhead
 */
template <typename T, void (*freefn)(T*)>
class safe_ptr
{
    T *ptr;

public:
    using value_type = T;
    using ptr_type = T *;

    inline safe_ptr(ptr_type ptr) :
        ptr(ptr)
    {}

    inline ~safe_ptr()
    {
        if (ptr)
            freefn(ptr);
    }

    inline ptr_type operator->()
    {
        return ptr;
    }

    inline value_type operator*()
    {
        return *ptr;
    }

    inline operator ptr_type ()
    {
        return ptr;
    }

    /*
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

    inline bool operator!()
    {
        return !ptr;
    }

    inline operator bool()
    {
        return ptr;
    }
};

}
}
