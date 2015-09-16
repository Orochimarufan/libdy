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
    const char *typeName() const;

    // Comparison
    /**
     * @brief Check if two Objects have the same identity
     * @param other The other object
     * @note Use the operators if you want to check for data equality
     */
    inline bool is(const Object &other) const;

    /**
     * @brief Check if two Objects are equal
     * @param other The other object
     * @note Equal means they contain the same data, in contrast to having
     *       the same identity
     * @sa is()
     * @warning This may or may not be implemented for all data types. At the very
     *          least, any two objects with the same identity are also equal.
     */
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
     * @return a SubscriptionRef of the item
     * @sa getItem()
     */
    SubscriptionRef operator[] (const Object &key) const;
    SubscriptionRef operator[] (DyObject *key) const;

    template <typename T>
    inline SubscriptionRef operator[] (T key) const;

    /**
     * @brief Get a member item
     * @param key The member key
     * @param defval The default
     * @return The item
     * @sa operator[]
     * @note Contrary to operator[], this will NOT return a SubscriptionRef.
     *       It also won't throw an exception if the key is not found.
     *       Instead, the specified default value is returned
     */
    Object getItem(const Object &key, const Object &defval) const;

    /**
     * @brief Get a member item
     * @param key The member key
     * @return The item
     * Overload of getItem(key, defval) where defval is Undefined
     */
    Object getItem(const Object &key) const;

    // Conversion
    template <typename T,
        typename = typename std::enable_if<!std::is_base_of<Object, T>::value>::type>
    operator T () const;

    /**
     * @brief Get the raw DyObject pointer
     * @return The DyObject pointer
     */
    inline DyObject *get() const;

    // Calling
    /**
     * @brief Call the object
     */
    Object operator()();
    Object operator()(const Object &);

    template <typename... Args>
    inline Object operator()(Args...);
};

// -----------------------------------------------------------------------------
// Type-specific subclasses
/**
 * @class String
 * @brief Class wrapping a libdy string
 */
class String : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    String(DyObject *object, bool steal=false);
    String(const Object &object);
    String(Object &&object);

    /**
     * @brief Create a String object
     */
    String(const char *c_str, std::size_t len);

    /**
     * @brief Get the 0-terminated bytestring pointer
     * @return The c-string pointer
     */
    const char *c_str();
};

/**
 * @class List
 * @brief Class wrapping a libdy list
 */
class List : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    List(DyObject *object, bool steal=false);
    List(const Object &object);
    List(Object &&object);

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
    void append(const Object &o);

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
    void insert(std::size_t at, const Object &o);

    /**
     * @brief
     */
    inline void extend(const List &other);

    /**
     * @brief Remove all items from the list
     */
    void clear();

    // Iterator (STL-compatible)
    class Iterator;
    typedef Iterator iterator;

    inline Iterator begin() const;
    inline Iterator end() const;
};

/**
 * @class Iterator
 * @brief STL-compatible iterator over a libdy list
 */
class List::Iterator
{
    List lst;
    std::size_t i;

public:
    inline Iterator(const List &lst, std::size_t i=0);

    inline Object operator*() const;

    inline Iterator & operator++();

    inline bool operator!=(const Iterator &other) const;
};

/**
 * @class Dict
 * @brief  Class wrapping a libdy dictionary
 */
class Dict : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    Dict(DyObject *object, bool steal=false);
    Dict(const Object &object);
    Dict(Object &&object);

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
    void clear();

    // Iterator
    class Iterator;

    inline Iterator iter();

    // STL iterator
    typedef Iterator iterator;

    inline Iterator begin();
    inline Iterator end();
};

/**
 * @class Iterator
 * @brief STL-compatible iterator over a libdy dictionary
 * @warning most methods don't check if the iterator is valid(). DO NOT try to
 *          use methods other than valid() and op!= on invalid iterators!
 */
class Dict::Iterator
{
public:
    struct Pair {
        DyObject *pkey;
        DyObject *pvalue;

        inline Object key();
        inline Object value();
    };

private:
    Pair **iter;

    Iterator();
    friend Iterator Dict::end();

public:
    Iterator(const Dict &dct);
    ~Iterator();

    inline Object key();
    inline Object value();

    bool next();
    inline bool valid();

    // STL compatibility
    inline const Pair &operator *();
    inline Iterator &operator++();
    inline bool operator !=(const Iterator &other);
};

class Userdata : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    Userdata(DyObject *object, bool steal=false);
    Userdata(const Object &object);
    Userdata(Object &&object);

    Userdata(void *ptr);
    Userdata(void *ptr, const char *name);

    void setDestructor(void(*fn)(void*));

    void *data() const;
    const char *name() const;
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

    // Calling
    Object operator()();

    Object operator()(const Object &arg);

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

    bool hasCause() const;
    Exception cause() const;

    void *data() const;

    DyObject *get() const;

    void clear();
};

}

#include "dy++impl.h"
