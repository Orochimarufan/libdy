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

#include "dy++.h"

#include <libdy/exceptions.h>

#include <initializer_list>

#ifdef __LIBDYPP_TRAP
#include <csignal>
#define __TRAP__ std::raise(SIGTRAP);
#else
#define __TRAP__
#endif

namespace Dy {

inline void Object::check(::DyObject *o)
{
    if (!o)
    {
    	::DyObject *e = ::DyErr_Occurred();
    	if (!e)
    		e = ::DyErr_Set("dy.cxx.NullPointer", "Tried to create null reference");
    	throw_exception(e);
    }
}

void Object::assign(::DyObject *o, bool steal)
{
    check(o);
    if (!steal)
    	::Dy_Retain(o);
    if (d)
    	::Dy_Release(d);
    d = o;
}

// [[ Object ]]
// Creation
Object::Object(DyObject *object, bool steal) :
    d(0)
{
    assign(object, steal);
}

Object::Object(const Object &object) :
    d(0)
{
    assign(object.d);
}

Object::Object(Object &&object) :
    d(0)
{
    assign(object.d, true);
    object.d = Dy_Retain(Dy_Undefined);
}

Object::Object() :
    d(Dy_Retain(Dy_Undefined))
{
}

Dict dict(std::initializer_list<std::pair<Object, Object>> il, Object parent)
{
    return Dict(il, parent);
}

// Destruction
Object::~Object()
{
    Dy_Release(d);
}

// Type
DyObjectType Object::type() const
{
    return Dy_Type(d);
}

const char *Object::type_name() const
{
    return ::Dy_GetTypeName(type());
}

// Length
size_t Object::length() const
{
    return Dy_Length(d);
}

// Representation
String Object::str() const
{
    return String(Dy_Str(d), true);
}

String Object::repr() const
{
    return String(Dy_Repr(d), true);
}

// Comparison
bool Object::operator==(const Object &other) const
{
    return Dy_Equals(d, other.d);
}

bool Object::operator!=(const Object &other) const
{
    return !Dy_Equals(d, other.d);
}

DyHash Object::hash()
{
    DyHash result;
    if (!Dy_HashEx(d, &result))
    	throw_exception(DY_ERRID_NOT_HASHABLE, "Object is not hashable");
    return result;
}

// Subscription
SubscriptionRef Object::operator[] (const Object &key)
{
    return SubscriptionRef(d, key.get());
}

SubscriptionRef Object::operator[] (::DyObject *key)
{
    return SubscriptionRef(d, key);
}

// Subtypes --------------------------------------------------------------------
#define SUBTYPE_CONSTRUCTORS(stname) \
stname::stname(DyObject *obj, bool steal) \
    : Object(obj, steal) \
{ \
    typecheck(obj); \
} \
stname::stname(const Object &obj) \
    : Object(obj) \
{ \
    typecheck(obj.get()); \
}
/*stname::stname(Object &&obj) \
{ \
    typecheck(obj.get()); \
    assign(obj.get(), true); \
    obj.d = Dy_Retain(Dy_Undefined); \
}*/

#define SUBTYPE_TYPECHECK(stname, want_type) \
void stname::typecheck(DyObject *obj) \
{ \
    if (Dy_Type(obj) != want_type) \
        format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Cannot create Dy::" # stname " from %s", Dy_GetTypeName(Dy_Type(obj))); \
}

// String
SUBTYPE_CONSTRUCTORS(String)
SUBTYPE_TYPECHECK(String, DY_STRING)

String::String(const char *c_str, std::size_t len) :
    Object(DyString_FromStringAndSize(c_str, len), true)
{}

// List
SUBTYPE_CONSTRUCTORS(List)
SUBTYPE_TYPECHECK(List, DY_LIST)

List::List(std::initializer_list<Object> il) :
    Object(DyList_NewEx(il.size()), true)
{
    for (decltype(il)::const_iterator i = il.begin(); i != il.end(); ++i)
        if (!DyList_Append(d, i->get()))
            throw_exception();
}

List::List() :
    Object(DyList_New(), true)
{}

List::List(std::size_t prealloc) :
    Object(DyList_NewEx(prealloc), true)
{}

// Dict
SUBTYPE_CONSTRUCTORS(Dict)
SUBTYPE_TYPECHECK(Dict, DY_DICT)

Dict::Dict(std::initializer_list<std::pair<Object, Object>> il, Object parent)
{
    DyObject *o;
    if (parent != Undefined)
        o = DyDict_NewWithParent(parent.get());
    else
        o = DyDict_New();

    for (decltype(il)::const_iterator i = il.begin(); i != il.end(); ++i)
        if (!Dy_SetItem(o, i->first.get(), i->second.get()))
        {
            Dy_Release(o);
            throw_exception();
        }

    assign(o, true);
}

Dict::Dict() :
    Object(DyDict_New(), true)
{}


// [[ SubscriptionRef ]]
SubscriptionRef::SubscriptionRef(DyObject *container, DyObject *key) :
    Object(Dy_GetItemU(container, key)),
    container(Dy_Retain(container)),
    key(Dy_Retain(key))
{
}

SubscriptionRef::~SubscriptionRef()
{
    Dy_Release(key);
    Dy_Release(container);
}

// Delete
void SubscriptionRef::del()
{
    if (!Dy_SetItem(container, key, nullptr))
        throw_exception();
    assign(Dy_Undefined);
}

// Assign
SubscriptionRef &SubscriptionRef::operator =(DyObject *object)
{
    assign(object);
    if (!Dy_SetItem(container, key, object))
        throw_exception();
    return *this;
}

SubscriptionRef &SubscriptionRef::operator =(const Object &object)
{
    assign(object.get());
    if (!Dy_SetItem(container, key, object.get()))
        throw_exception();
    return *this;
}

SubscriptionRef &SubscriptionRef::operator =(Object &&object)
{
    assign(object.d, true);
    object.d = Dy_Retain(Dy_Undefined);
    if (!Dy_SetItem(container, key, d))
        throw_exception();
    return *this;
}

// [[ Exception ]]
Exception::Exception(DyObject *exception) :
    d(Dy_Retain(exception))
{
}

Exception::~Exception() noexcept
{
    Dy_Release(d);
}

const char *Exception::what()
{
    return DyErr_Message(d);
}

const char *Exception::errid() const
{
    return DyErr_ErrId(d);
}

const char *Exception::message() const
{
    return DyErr_Message(d);
}

DyObject *Exception::cause() const
{
    return DyErr_Cause(d);
}

void *Exception::data() const
{
    return DyErr_Data(d);
}

void Exception::clear()
{
    if (DyErr_Occurred() == d)
    	DyErr_Clear();
}

// Error
void throw_exception()
{
    ::DyObject *e = DyErr_Occurred();
    if (!e)
        e = DyErr_Set("cxx.throw.NoExceptionSet", "throw_exception() called with no libdy exception set");
    __TRAP__
    throw Exception(e);
}

void throw_exception(DyObject *exception)
{
    if (!exception)
        exception = DyErr_Set("cxx.throw.NullException", "NULL passed to throw_exception(exc)");
    else if (DyErr_Occurred() != exception)
        DyErr_SetObject(exception);
    __TRAP__
    throw Exception(exception);
}

void throw_exception(const char *errid, const char *message)
{
    ::DyObject *e = DyErr_Set(errid, message);
    __TRAP__
    throw Exception(e);
}

void format_exception(const char *errid, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    ::DyObject *e = DyErr_FormatV(errid, format, va);
    va_end(va);
    __TRAP__
    throw Exception(e);
}

// Constants
const Object Undefined(Dy_Undefined);
const Object None(Dy_None);
const Object True(Dy_True);
const Object False(Dy_False);

}
