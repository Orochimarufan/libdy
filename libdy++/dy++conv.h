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

#include "util.h"

#include <libdy/constants.h>
#include <libdy/numbers.h>
#include <libdy/dystring.h>

#include <string>


/**
 * @file dy++conv.h
 * @ingroup Conversion
 * @brief Built-in conversions for C++ types
 * @sa dy++qt.h
 * @note This includes much of the C libdy API
 */

namespace Dy {
namespace conv {

// ===================================================================
// Simple values

/// @brief conversion for bool
template <>
struct convert<bool> {
    static inline DyObject *from_value(const bool &val)
    {
        return Dy_Retain(val ? Dy_True : Dy_False);
    }
    static inline bool to_value(DyObject *obj)
    {
        if (!DyBool_Check(obj))
            format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Cannot convert Dy::%s to bool", Dy_GetTypeName(Dy_Type(obj)));
        return obj == Dy_True;
    }
};

/// @brief conversion from int
template <>
struct convert<int> {
    static inline DyObject *from_value(int val)
    {
        return DyLong_New(val);
    }
};

/// @brief conversion from unsigned int
template <>
struct convert<unsigned> {
    static inline DyObject *from_value(unsigned number)
    {
        return DyLong_New(number);
    }
};

/// @brief conversion for long
template <>
struct convert<long> {
    static inline DyObject *from_value(long number)
    {
        return DyLong_New(number);
    }
    static inline long to_value(DyObject *o)
    {
        if (!DyLong_Check(o))
            throw_exception("dy.TypeError.CXXConversionError", "Could not convert to long");

        return DyLong_Get(o);
    }
};

/// @brief conversion from unsigned long
/// @warning This may incur data loss
/// @todo possibly remove this because of the data loss
template <>
struct convert<unsigned long> {
    static inline DyObject *from_value(unsigned long number)
    {
        return DyLong_New(number);
    }
};

/// @brief conversion for double
template <>
struct convert<double> {
    static inline DyObject *from_value(double floating)
    {
        return DyFloat_New(floating);
    }
    static inline double to_value(DyObject *o)
    {
        if (!DyFloat_Check(o))
            throw_exception("dy.TypeError.CXXConversionError", "Could not convert to float");

        return DyFloat_Get(o);
    }
};

/// @brief conversion for const char * (C strings)
template <>
struct convert<const char *> {
    static inline DyObject *from_value(const char *str)
    {
        return DyString_FromString(str);
    }
    static inline const char *to_value(DyObject *o)
    {
        if (!DyString_Check(o))
            throw_exception("dy.TypeError.CXXConversionError", "Could not convert to string");

        // NOTE: this is NOT copied, so you don't need to free it
        // BUT it also has the same lifetime as the string object
        return DyString_AsString(o);
    }
};

/// @brief conversion from std::string
template <>
struct convert<std::string> {
    static inline DyObject *from_value(const std::string &str)
    {
        return DyString_FromStringAndSize(str.c_str(), str.size());
    }
};

}
}
