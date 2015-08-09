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

#include <libdy/dy.h>

#include <string>

namespace Dy {

void throw_exception(const char *errid, const char *message);

namespace conv {

// ===================================================================
// From Value
inline DyObject *from_value(long number)
{
    return DyLong_New(number);
}

//inline DyObject *from_value(double floating)
//{
//    return DyFloat_New(floating);
//}

inline DyObject *from_value(const char *str)
{
    return DyString_FromString(str);
}

inline DyObject *from_value(const std::string &str)
{
    return DyString_FromStringAndSize(str.c_str(), str.size());
}

// From Value or Reference
template <typename T>
inline DyObject *from_value_or_ref(T value)
{
    return from_value(value);
}

template <>
inline DyObject *from_value_or_ref(DyObject *o)
{
    return Dy_Retain(o);
}

template <>
inline DyObject *from_value_or_ref(const Object &o)
{
    return Dy_Retain(o.get());
}

// ===================================================================
// To Value
template <typename T>
T to_value(DyObject *);

template <>
inline double to_value(DyObject *o)
{
    if (!DyFloat_Check(o))
    	throw_exception("dy.TypeError.CXXConversionError", "Could not convert to float");
    
    return DyFloat_Get(o);
}

template <>
inline long to_value(DyObject *o)
{
    if (!DyLong_Check(o))
    	throw_exception("dy.TypeError.CXXConversionError", "Could not convert to long");

    return DyLong_Get(o);
}

template <>
inline const char *to_value(DyObject *o)
{
    if (!DyString_Check(o))
    	throw_exception("dy.TypeError.CXXConversionError", "Could not convert to string");
    
    // NOTE: this is NOT copied, so you don't need to free it
    // BUT it also has the same lifetime as the string object
    return DyString_AsString(o);
}

}
}
