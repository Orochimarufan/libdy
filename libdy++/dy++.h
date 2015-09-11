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

#include <libdy/types.h>

#include <exception>
#include <initializer_list>
#include <utility>

namespace Dy {

class Object;

class String;
class List;
class Dict;

class SubscriptionRef;
class Exception;

extern const Object Undefined;
extern const Object None;
extern const Object True;
extern const Object False;

// -----------------------------------------------------------------------------
// Base Object class
/**
 * @brief Object Reference class.
 * @warning Polymorphism is NOT supported.
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
     * @brief Create a reference to Undefined
     */
    Object();

    /**
     * @brief Create an Object reference from value
     */
    template <typename T,
        typename = typename std::enable_if<!std::is_base_of<Object, T>::value>::type>
    Object(T value);

    ~Object();

    // Type
    DyObjectType type() const;
    const char *type_name() const;

    // Comparison
    bool operator==(const Object &) const;
    bool operator!=(const Object &) const;

    DyHash hash();

    // Representation
    String str() const;
    String repr() const;

    // Length
    std::size_t length() const;

    // Subscription
    /**
     * @brief Subscribe to an object
     * @param key The member key
     */
    SubscriptionRef operator[] (const Object &key);
    SubscriptionRef operator[] (DyObject *key);

    template <typename T>
    SubscriptionRef operator[] (T);

    // Conversion
    template <typename T>
    operator T () const;

    inline DyObject *get() const;

    // Calling
    inline Object operator()();

    template <typename Arg>
    inline Object operator()(Arg);

    template <typename... Args>
    inline Object operator()(Args...);
};

// -----------------------------------------------------------------------------
// Type-specific subclasses
class String : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    String(DyObject *object, bool steal=false);
    String(const Object &object);
    //String(Object &&object);

    /**
     * @brief Create a String object
     */
    String(const char *c_str, std::size_t len);
};

class List : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    List(DyObject *object, bool steal=false);
    List(const Object &object);
    //List(Object &&object);

    // Initializer-construction
    /**
     * @brief Construct a libdy list
     */
    List(std::initializer_list<Object> il);

    /**
     * @brief Create an empty list
     */
    List();

    /**
     * @brief Create an empty list
     * @param prealloc Pre-allocate this many items
     * @sa List()
     */
    List(std::size_t prealloc);

    /**
     * @brief Make a list from all arguments
     */
    template <typename... Items>
    inline static List make(Items...);

    /**
     * @brief Append an object to the list
     * @param o The object
     */
    inline void append(const Object &o);

    /**
     * @brief Append all items to the list
     */
    template <typename Item, typename... Tail>
    inline void appendMany(Item, Tail...);

    template <typename Item>
    inline void appendMany(Item);

    /**
     * @brief Insert an object into the list
     * @param at The position to insert at
     * @param o The Object
     */
    inline void insert(std::size_t at, const Object &o);

    /**
     * @brief Remove all items from the list
     */
    inline void clear();
};

class Dict : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    Dict(DyObject *object, bool steal=false);
    Dict(const Object &object);
    //Dict(Object &&object);

    /**
     * @brief Create a new dictionary
     * @param il Pass a initializer_list of items
     * @param parent A dictionary can inherit from another dictionary
     */
    Dict(std::initializer_list<std::pair<Object, Object>> il, Object parent=Undefined);

    /**
     * @brief Create an empty dictionary
     */
    Dict();

    /**
     * @brief Remove all entries from the dictionary
     */
    inline void clear();

    class Iterator
    {
    public:
        struct Pair {
            DyObject *key;
            DyObject *value;
        };

    private:
        Pair **iter;

    public:
        inline Iterator(const Dict &dct);

        inline Pair *operator *();

        inline Object key();
        inline Object value();

        inline bool next();
        inline Iterator &operator++();

        inline ~Iterator();
    };

    inline Iterator iter();
};

// -----------------------------------------------------------------------------
// Special classes
/**
 * @brief Reference retrieved by subscribing to another object
 * This allows assignments like object["hello"] = "Derp";
 * @note These should be treated as temporary objects only.
 *       Storing them (SubscriptionRef x = object["hello"]) does not make sense
 *       Be careful about auto x = object["hello"] because it might defer
 *       SubscriptionRef as the resulting type.
 *       Object/String/List/Dict x = object["hello"] should be used instead.
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

    // Calling
    inline Object operator()();

    template <typename Arg>
    inline Object operator()(Arg);

    template <typename... Args>
    inline Object operator()(Args...);
};

class Exception : std::exception
{
    DyObject *d;

    Exception(DyObject *exception);

    friend void throw_exception();
    friend void throw_exception(DyObject *);
    friend void throw_exception(const char *, const char *);
    friend void format_exception(const char *, const char *, ...);

public:
    ~Exception() noexcept;

    const char *what();

    const char *errid() const;
    const char *message() const;
    DyObject *cause() const;
    void *data() const;

    void clear();
};

}

#include "dy++impl.h"
