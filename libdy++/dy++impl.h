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

#include <libdy/object.h> // TODO: keep the C symbols separate?
#include <libdy/collections.h>
#include "dy++conv.h" // TODO: maybe make this opt-in?

// Included from dypp.h


namespace Dy {

// Generic functions
template <typename First, typename... More>
inline void appendToList(Object &list, First arg, More... args)
{
    DyList_Append(list.get(), Dy_Pass(conv::from_value_or_ref(arg)));

    appendToList(list, args...);
}

template <typename First>
inline void appendToList(Object &list, First arg)
{
    DyList_Append(list.get(), Dy_Pass(conv::from_value_or_ref(arg)));
}

template <typename... Args>
inline Object makeList(Args... args)
{
    Object list = Object(DyList_New(), true);

    appendToList(list, args...);

    return list;
}

// -----------------------------------------------------------------------------
// Object

// Call operator
inline Object Object::operator()()
{
    return Object(DyCallable_Call0(get(), NULL), true);
}

template <typename Arg>
inline Object Object::operator()(Arg arg)
{
    return Object(DyCallable_Call1(get(), NULL, Object(arg).get()), true); // TODO: try to use from_value instead of Object().get()
}

template <typename... Args>
inline Object Object::operator()(Args... args)
{
    return Object(Dy_Call(get(), NULL, makeList(args...).get()), true);
}

// Constructor
template <typename T>
inline Object::Object(T value) :
    d(0)
{
    assign(conv::from_value(value));
}

template <typename T>
inline Object::operator T () const
{
    return conv::to_value<T>(d);
}

inline DyObject *Object::get() const
{
    return d;
}

// -----------------------------------------------------------------------------
// SubscriptionRef
template <typename T>
inline SubscriptionRef Object::operator[] (T key)
{
    return SubscriptionRef(d, ::Dy_Pass(conv::from_value(key)));
}

template <typename T>
SubscriptionRef &SubscriptionRef::operator =(T value)
{
    assign(conv::from_value(value), true);
    Dy_SetItem(container, key, d);
    return *this;
}

// Call operator (with self)
inline Object SubscriptionRef::operator()()
{
    return Object(DyCallable_Call0(get(), container), true);
}

template <typename Arg>
inline Object SubscriptionRef::operator()(Arg arg)
{
    return Object(DyCallable_Call1(get(), container, Object(arg).get()), true); // TODO: try to use from_value instead of Object().get()
}

template <typename... Args>
inline Object SubscriptionRef::operator()(Args... args)
{
    return Object(Dy_Call(get(), container, makeList(args...).get()), true);
}

}
