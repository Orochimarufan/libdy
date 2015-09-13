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

#include <type_traits>

// Included from dypp.h


namespace Dy {

namespace util {
    using safe_dy_ptr = safe_ptr<DyObject, Dy_Release>;
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
    return Object(DyCallable_Call1(get(), NULL, Object(arg).get()), true);
}

template <typename... Args>
inline Object Object::operator()(Args... args)
{
    return Object(Dy_Call(get(), NULL, List::make(args...).get()), true);
}

// Constructor
template <typename T, typename>
Object::Object(T value) :
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

// Comparison
inline bool Object::is(const Object &other) const
{
    return d == other.d;
}

// -----------------------------------------------------------------------------
// List
inline void List::append(const Object &o)
{
    if (!DyList_Append(d, o.get()))
        throw_exception();
}

inline void List::insert(std::size_t at, const Object &o)
{
    if (!DyList_Insert(d, at, o.get()))
        throw_exception();
}

inline void List::extend(const List &other)
{
    for (auto it : other)
        append(it);
}

template <typename First, typename... More>
inline void List::appendMany(First arg, More... args)
{
    append(arg);
    appendAll(args...);
}

template <typename First>
inline void List::appendMany(First arg)
{
    append(arg);
}

template <typename... Args>
inline List List::make(Args... args)
{
    List list;

    list.appendMany(args...);

    return list;
}

inline void List::clear()
{
    if (!DyList_Clear(d))
        throw_exception();
}

// Iterator
inline List::Iterator::Iterator(const List &lst, std::size_t i) :
    lst(lst), i(i)
{}

inline Object List::Iterator::operator *() const
{
    return lst[i];
}

inline List::Iterator &List::Iterator::operator ++()
{
    ++i;
    return *this;
}

inline bool List::Iterator::operator !=(const List::Iterator &other) const
{
    return !lst.is(other.lst) || i != other.i;
}

inline List::Iterator List::begin() const
{
    return List::Iterator(*this, 0);
}

inline List::Iterator List::end() const
{
    return List::Iterator(*this, length());
}

// -----------------------------------------------------------------------------
// Dict
inline void Dict::clear()
{
    if (!DyDict_Clear(d))
        throw_exception();
}

inline Dict::Iterator::Iterator(const Dict &dct) :
    iter(reinterpret_cast<Dict::Iterator::Pair**>(DyDict_Iter(dct.get())))
{
    if (!iter)
        throw_exception();
}

inline Dict::Iterator::Pair *Dict::Iterator::operator *()
{
    return *iter;
}

inline Object Dict::Iterator::key()
{
    return (*iter)->key;
}

inline Object Dict::Iterator::value()
{
    return (*iter)->value;
}

inline bool Dict::Iterator::next()
{
    return DyDict_IterNext(reinterpret_cast<DyDict_IterPair**>(iter));
}

inline Dict::Iterator &Dict::Iterator::operator++()
{
    DyDict_IterNext(reinterpret_cast<DyDict_IterPair**>(iter));
    return *this;
}

inline Dict::Iterator::~Iterator()
{
    DyDict_IterFree(reinterpret_cast<DyDict_IterPair**>(iter));
}

inline Dict::Iterator Dict::iter()
{
    return Dict::Iterator(*this);
}

// -----------------------------------------------------------------------------
// SubscriptionRef
template <typename T>
inline SubscriptionRef Object::operator[] (T key) const
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
    return Object(DyCallable_Call1(get(), container, Object(arg).get()), true);
}

template <typename... Args>
inline Object SubscriptionRef::operator()(Args... args)
{
    return Object(Dy_Call(get(), container, List::make(args...).get()), true);
}

}
