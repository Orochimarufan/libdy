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

#pragma once // File included from dy++.h

//#include "dy++conv.h"

namespace Dy {

namespace detail {
    Object Call(Object &, DyObject *, const List &);
    DyObject *RGetItemLong(DyObject *, long);
}

// -----------------------------------------------------------------------------
// Object

// Call operator
template <typename... Args>
inline Object Object::operator()(Args... args)
{
    return detail::Call(*this, nullptr, List::make(args...));
}

// Constructor
template <typename T, typename>
Object::Object(T value) :
    d(0)
{
    assign(conv::from_value(value));
}

template <typename T, typename>
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

// Subscription
template <typename T>
inline SubscriptionRef Object::operator [](T key) const
{
    return this->operator[](conv::from_value(key));
}

// -----------------------------------------------------------------------------
// List
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
inline Object Dict::Iterator::key()
{
    return (*iter)->pkey;
}

inline Object Dict::Iterator::value()
{
    return (*iter)->pvalue;
}

inline bool Dict::Iterator::valid()
{
    return iter && *iter;
}

inline Object Dict::Iterator::Pair::key()
{
    return Object(pkey);
}

inline Object Dict::Iterator::Pair::value()
{
    return Object(pvalue);
}

inline const Dict::Iterator::Pair &Dict::Iterator::operator *()
{
    return **iter;
}

inline Dict::Iterator::Iterator() :
    iter(nullptr)
{}

inline Dict::Iterator &Dict::Iterator::operator++()
{
    next();
    return *this;
}

inline bool Dict::Iterator::operator !=(const Dict::Iterator &other)
{
    // HAX: basically, if the validity or the pointed-to Pair differs.
    return !!iter ? (!!other.iter ? *iter != *other.iter : !!*iter) : !!other.iter && !!*other.iter;
}

inline Dict::Iterator Dict::iter()
{
    return Dict::Iterator(*this);
}

inline Dict::Iterator Dict::begin()
{
    return Dict::Iterator(*this);
}

inline Dict::Iterator Dict::end()
{
    return Dict::Iterator();
}

// -----------------------------------------------------------------------------
// SubscriptionRef
// Call operator (with self)
template <typename... Args>
inline Object SubscriptionRef::operator()(Args... args)
{
    return detail::Call(*this, container, List::make(args...));
}

// -----------------------------------------------------------------------------
// Exception
inline DyObject *Exception::get() const
{
    return d;
}

}
