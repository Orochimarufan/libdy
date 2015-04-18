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

#include <exception>

namespace Dy {

class Object;
class SubscriptionRef;
class Exception;

/**
 * @brief Object Reference class.
 */
class Object
{
protected:
    DyObject *d;

    friend class SubscriptionRef;

    void assign(DyObject *, bool steal=false);
    static void check(DyObject *);

public:
    // Construction
    /**
     * @brief Create an Object reference from raw pointer
     * @param object the object pointer
     * @param steal steal the reference [default: false]
     */
    Object(DyObject *object, bool steal=false);

    /**
     * @brief Copy an Object reference
     */
    Object(const Object &object);

    /**
     * @brief Move an Object reference
     */
    Object(Object &&object);

    /**
     * @brief Create an Object reference from value
     */
    template <typename T>
    Object(T value);

    ~Object();

    // Type
    DyObject_Type type() const;
    const char *type_name() const;

    // Subscription
    /**
     * @brief Subscribe to an object
     * @param key The member key
     */
    SubscriptionRef operator[] (const Object &key);
    SubscriptionRef operator[] (DyObject *key);

    template <typename T>
    SubscriptionRef operator[] (T);

    // Length
    std::size_t length() const;

    // Comparison
    bool operator==(const Object &);
    bool operator!=(const Object &);

    Dy_hash_t hash();

    // Representation
    Object str() const;
    Object repr() const;

    // Conversion
    template <typename T>
    operator T () const;

    inline DyObject *get() const;
    
    // Calling
    template <typename Arg>
    inline Object operator()(Arg);
    
    template <typename... Args>
    inline Object operator()(Args...);
};

/**
 * @brief Reference retrieved by subscribing to another object
 * This allows assignments like object["hello"] = "Derp";
 * NOTE These should be treated as temporary objects only.
 *     	Storing them (SubscriptionRef x = object["hello"]) does not make sense
 *     	Be careful about auto x = object["hello"] because it might defer SubscriptionRef
 *     	as the resulting type. Object x = object["hello"] is preferred.
 * WARNING The Destructor is non-virtual, so polymorphism mustn't be relied on.
 */
class SubscriptionRef : public Object
{
    friend class Object;

    DyObject *container;
    DyObject *key;

    SubscriptionRef(DyObject *container, DyObject *key);

public:
    ~SubscriptionRef();

    void del();

    SubscriptionRef &operator =(::DyObject *object);
    SubscriptionRef &operator =(const Object &object);
    SubscriptionRef &operator =(Object &&object);

    template <typename T>
    SubscriptionRef &operator =(T value);
};

class Exception : std::exception
{
    DyObject *d;

public:
    Exception(DyObject *exception);
    ~Exception() noexcept;

    const char *what();

    const char *errid() const;
    const char *message() const;
    DyObject *cause() const;
    void *data() const;

    void clear();
};

// data is optional. see dycpp_conv.h
void throw_exception(const char *errid, const char *message, void *data);

template <typename... Args>
inline Object makeList(Args... args);

template <typename... Args>
inline void appendToList(Object &list, Args... args);

}

#include "dypp.impl.h"
