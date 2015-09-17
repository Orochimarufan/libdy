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


/**
 * @file dy++.h
 * @brief libdy++'s main include file
 * This file contains all basic types of libdy++
 */

namespace Dy {

// Forward Declarations
class Object;

class String;
class List;
class Dict;

class SubscriptionRef;
class Exception;

// -----------------------------------------------------------------------------
// Constants
/// @addtogroup Constants
/// @{

/// @brief The Undefined constant
/// This constant is to be seen as libdy's equivalent to NULL
extern const Object Undefined;

/// @brief The None constant
extern const Object None;

/// @brief The boolean True constant
extern const Object True;

/// @brief The boolean False constant
extern const Object False;

/// @}

// -----------------------------------------------------------------------------
// Base Object class
/**
 * @class Object
 * @ingroup Objects
 * @brief Object Reference class.
 * @warning Polymorphism is NOT supported.
 */
class Object
{
protected:
    /// @cond
    // Not part of the public api
    DyObject *d;

    friend class SubscriptionRef;

    void assign(DyObject *, bool steal=false);
    static void check(DyObject *);
    /// @endcond

public:
    // Construction
    /**
     * @brief Create an Object reference from raw pointer
     * @param object the object pointer
     * @param steal steal the reference
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
    /**
     * @brief Get the libdy object type
     * @return The libdy object type
     */
    DyObjectType type() const;

    /**
     * @brief Get the object's type name
     * @return The object's type name
     */
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
    bool operator==(const Object &other) const;

    /**
     * @brief Check if two Objects are inequal
     * @param other The other object
     * @sa operator==
     * @warning This may not be implemented for all types, so this might return
     *          true for actually equal objects, but not vice versa.
     */
    bool operator!=(const Object &other) const;

    /**
     * @brief Generate a hash value for this object
     * @return A non-cryptographic hash
     */
    DyHash hash();

    // Representation
    /**
     * @brief Get the string(-ified) version of the object
     * @return A String
     */
    String str() const;

    /**
     * @brief Get a string representation of the object
     * @return A String
     */
    String repr() const;

    // Length
    /**
     * @brief Get the length of the container or string
     * @return The length
     * @throws Exception if this object is neither a container nor a string
     */
    std::size_t length() const;

    // Subscription
    /**
     * @brief Subscribe to an object
     * @param key The member key
     * @return a SubscriptionRef of the item
     * @sa getItem()
     */
    SubscriptionRef operator[] (const Object &key) const;

    /**
     * @overload
     * @brief Subscribe to an object
     * @param key The member key
     * @return a SubscriptionRef of the item
     * @sa getItem()
     */
    SubscriptionRef operator[] (DyObject *key) const;

    /**
     * @overload
     * @brief Subscribe to an object
     * @param key The member key
     * @return a SubscriptionRef of the item
     * @sa getItem()
     */
    template <typename T>
    inline SubscriptionRef operator[] (T key) const;

    /**
     * @brief Get a member item
     * @param key The member key
     * @param defval The default
     * @return The item
     * @sa operator[]
     * @note Contrary to operator[], this will NOT return a SubscriptionRef.
     *       the specified default value is returned if key isn't found
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
    /**
     * @brief Conversion operator for implicit conversion to native types
     * @return A native object
     */
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
     * @note This will <b>NOT</b> supply the 'self' argument to method-type userdata objects
     */
    Object operator()();

    /**
     * @brief Call the object
     * @note This will <b>NOT</b> supply the 'self' argument to method-type userdata objects
     */
    Object operator()(const Object &);

    /**
     * @brief Call the object
     * @note This will <b>NOT</b> supply the 'self' argument to method-type userdata objects
     */
    template <typename... Args>
    inline Object operator()(Args...);
};

// -----------------------------------------------------------------------------
// Type-specific subclasses
/**
 * @class String
 * @ingroup Objects
 * @brief Class wrapping a libdy string
 */
class String : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    /// @brief Wrap a raw pointer
    String(DyObject *object, bool steal=false);
    /// @brief Copy constructor
    String(const Object &object);
    /// @brief Move constructor
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
 * @ingroup Objects
 * @brief Class wrapping a libdy list
 */
class List : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    /// @brief Wrap a raw pointer
    List(DyObject *object, bool steal=false);
    /// @brief Copy constructor
    List(const Object &object);
    /// @brief Move constructor
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
     * @brief Append all arguments to the list
     */
    template <typename Item, typename... Tail>
    inline void appendMany(Item, Tail...);

    /**
     * @brief Append all arguments to the list
     */
    template <typename Item>
    inline void appendMany(Item);

    /**
     * @brief Insert an object into the list
     * @param at The position to insert at
     * @param o The Object
     */
    void insert(std::size_t at, const Object &o);

    /**
     * @brief Extend this list with all elements of another
     * @param other The other list
     */
    inline void extend(const List &other);

    /**
     * @brief Remove all items from the list
     */
    void clear();

    // Iterator (STL-compatible)
    class Iterator;
    /// @brief A STL-compatible iterator type
    typedef Iterator iterator;

    /**
     * @brief Get an iterator at the beginning of the list
     */
    inline Iterator begin() const;

    /**
     * @brief Get an iterator past the end of the list
     */
    inline Iterator end() const;
};

/**
 * @class List::Iterator
 * @ingroup Iterators
 * @brief STL-compatible iterator over a libdy list
 */
class List::Iterator
{
    List lst;
    std::size_t i;

public:
    /**
     * @brief Create an iterator over a list
     * @param lst The list to iterate over
     * @param i The index to start iterating at
     */
    inline Iterator(const List &lst, std::size_t i=0);

    /**
     * @brief Retrieve the current item
     */
    inline Object operator*() const;

    /**
     * @brief Advance the iterator
     */
    inline Iterator & operator++();

    /**
     * @brief Compare two iterators for inequality
     * @param other The other iterator
     */
    inline bool operator!=(const Iterator &other) const;
};

/**
 * @class Dict
 * @ingroup Objects
 * @brief Class wrapping a libdy dictionary
 */
class Dict : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    /// @brief Wrap a raw pointer
    Dict(DyObject *object, bool steal=false);
    /// @brief Copy constructor
    Dict(const Object &object);
    /// @brief Move constructor
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

    /**
     * @brief Create an Iterator over this dictionary
     * @return An interator over this dictionary
     * @sa begin
     */
    inline Iterator iter();

    // STL iterator
    /// @brief STL-compatible iterator type
    typedef Iterator iterator;

    /**
     * @brief Create a STL-compatible iterator over this dictionary
     * @return An iterator over this dictionary
     * @sa end
     * @sa iter
     */
    inline Iterator begin();

    /**
     * @brief Create a STL-compatible iterator over this dictionary, positioned
     *        past the end
     * @return An iterator pointing past the end of this dictionary
     * @sa begin
     */
    inline Iterator end();
};

/**
 * @class Dict::Iterator
 * @ingroup Iterators
 * @brief STL-compatible iterator over a libdy dictionary
 * @warning most methods don't check if the iterator is valid(). DO NOT try to
 *          use methods other than valid() and op!= on invalid iterators!
 */
class Dict::Iterator
{
public:
    /**
     * @class Pair
     * @brief A key/value pair in the dictionary
     * @note The raw pointers are exposed because of the underlying libdy
     *       dictionary iterator protocol.
     */
    struct Pair {
        /// @brief The raw key
        DyObject *pkey;

        /// @brief The raw value
        DyObject *pvalue;

        /// @brief Get the key as Object
        inline Object key();

        /// @brief Get the value as Object
        inline Object value();
    };

private:
    Pair **iter;

    Iterator();
    friend Iterator Dict::end();

public:
    /**
     * @brief Create an interator over a dictionary
     * @param dct The dictionary
     */
    Iterator(const Dict &dct);
    ~Iterator();

    /**
     * @brief Get the current key
     * @return The key
     */
    inline Object key();

    /**
     * @brief Get the current value
     * @return The value
     */
    inline Object value();

    /**
     * @brief Advance the iterator by one
     * @return Whether the iterator is still valid
     */
    bool next();

    /**
     * @brief Check whether the iterator is valid (= is not past the end)
     */
    inline bool valid();

    // STL compatibility
    /**
     * @brief STL-compatible iterator dereferencing
     */
    inline const Pair &operator *();

    /**
     * @brief STL-compatible iterator advancing
     */
    inline Iterator &operator++();

    /**
     * @brief STL-compatible iterator inequality check
     * @param other Another, possibly past the end, iterator
     */
    inline bool operator !=(const Iterator &other);
};

/**
 * @class Userdata
 * @ingroup Objects
 * @brief Class wrapping a libdy userdata
 */
class Userdata : public Object
{
    static void typecheck(DyObject *);

public:
    // Construction
    /// @brief Wrap a raw pointer
    Userdata(DyObject *object, bool steal=false);
    /// @brief Copy constructor
    Userdata(const Object &object);
    /// @brief Move constructor
    Userdata(Object &&object);

    /**
     * @brief Create an %Userdata to wrap a raw pointer
     * @param ptr The raw pointer
     */
    Userdata(void *ptr);

    /**
     * @brief Create a named %Userdata wrapping a raw pointer
     * @param ptr The raw pointer
     * @param name The userdata name
     * @note The name must stay valid for the lifetime of the userdata
     */
    Userdata(void *ptr, const char *name);

    /**
     * @brief Set a function to call when the userdata gets garbage collected
     * @param fn A function taking a pointer to free
     */
    void setDestructor(void(*fn)(void*));

    /**
     * @brief Retrieve the raw data pointer from the userdata
     */
    void *data() const;

    /**
     * @brief Retrieve the userdata name
     * @return The userdata name or NULL
     */
    const char *name() const;
};


// -----------------------------------------------------------------------------
// Special classes
/**
 * @class SubscriptionRef
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

    /**
     * @brief Remove the item from the container
     */
    void del();

    /**
     * @brief Assign a new value to the item in the container
     * @param object The new value
     */
    SubscriptionRef &operator =(const Object &object);

    /**
     * @brief Move-assign a new value to the item in the container
     * @param object The new value
     */
    SubscriptionRef &operator =(Object &&object);

    // Calling
    /**
     * @brief Call the member
     * @note This will supply the 'self' argument to method-type userdata objects
     * @sa Object::operator()()
     */
    Object operator()();

    /**
     * @brief Call the member
     * @param arg An argument
     * @note This will supply the 'self' argument to method-type userdata objects
     * @sa Object::operator()(const Object &arg)
     */
    Object operator()(const Object &arg);

    /**
     * @brief Call the member
     * @note This will supply the 'self' argument to method-type userdata objects
     * @sa Object::operator()(Args...)
     */
    template <typename... Args>
    inline Object operator()(Args...);
};

/**
 * @ingroup Exceptions
 * @class Exception
 * @brief Class wrapping a libdy exception
 * @note This does NOT inherit Object. Instead, it derives from std::exception.
 */
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

    /**
     * @brief Get the error message
     * @sa message
     */
    const char *what();

    /**
     * @brief Get the libdy exception ID
     */
    const char *errid() const;

    /**
     * @brief Get the error message
     */
    const char *message() const;

    /**
     * @brief Check if this exception was caused by another one
     */
    bool hasCause() const;

    /**
     * @brief Retrieve the cause of this exception
     * @return
     */
    Exception cause() const;

    /**
     * @brief Retrieve the data pointer attached to the exception
     */
    void *data() const;

    /**
     * @brief Get the raw libdy exception pointer
     */
    DyObject *get() const;

    /**
     * @brief Clear the libdy exception state
     * @warning This method <b>MUST</b> be called when catching and not re-throwing
     *          a libdy++ exception!
     */
    void clear();
};

}

#include "dy++impl.h"
