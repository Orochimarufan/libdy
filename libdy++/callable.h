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

#include "dy++.h"

#include <libdy/constants.h>
#include <libdy/object.h>
#include <libdy/userdata.h>
#include <libdy/exceptions.h>

#include <functional>
#include <tuple>
#include <utility>
#include <type_traits>


/**
 * @file callable.h
 * @brief Support code for creating callable libdy userdata objects from C++ functions
 */


namespace Dy {
namespace Callable {

/// @cond

// Type conversions
template <typename T>
inline T make_argument(DyObject *o)
{
    return Dy::conv::to_value<T>(o);
}

template <>
inline Object make_argument(DyObject *o)
{
    return o;
}

template <>
inline DyObject *make_argument(DyObject *o)
{
    return o;
}

template <typename T>
inline DyObject *make_return(T o)
{
    return Dy::conv::from_value(o);
}

inline DyObject *make_return(DyObject *o)
{
    return o;
}

inline DyObject *make_return(const Object &o)
{
    return Dy_Retain(o.get());
}

// magic
template <typename Return, typename... Args>
struct fnx {

    typedef std::function<Return(Args...)> function;

    typedef Return return_type;

    template<std::size_t N> using NthType =
        typename std::tuple_element<N, std::tuple<Args...>>::type;

    template<std::size_t N> using NthArgType =
        typename std::remove_cv<typename std::remove_reference<NthType<N>>::type>::type;

    static void free_function(void *fnp)
    {
        delete reinterpret_cast<function *>(fnp);
    }

    static inline DyObject *make_udata(DyUser_Callback tp, function *fnp)
    {
        DyObject *ud = DyUser_CreateCallable(tp, fnp);

        if (!ud)
            throw_exception();

        if (!DyUser_SetDestructor(ud, free_function))
            throw_exception();

        return ud;
    }

    template <std::size_t i>
    static inline NthArgType<i> get_arg(DyObject *args)
    {
        return make_argument<NthArgType<i>>(Dy_Pass(detail::RGetItemLong(args, i)));
    }

    template <std::size_t i>
    static inline NthArgType<i> get_arg_o(DyObject *args)
    {
        return make_argument<NthArgType<i + 1>>(Dy_Pass(detail::RGetItemLong(args, i)));
    }

    // Helper macros
#define MAKE(n) \
    template <std::size_t... I> \
    static inline DyObject *create ##n (function *fn, std::index_sequence<I...>) \
    { \
        return make_udata(reinterpret_cast<DyUser_Callback>(&call ##n<I...>), fn); \
    } \
    template <std::size_t... I> \
    static DyObject *call ##n(DyObject *self, function *fn, DyObject *args)

#define PROTECT(what) \
    try \
    { \
        what \
    } \
    catch (Dy::Exception &e) \
    { \
        return nullptr; \
    }

#define PROTECT_RETURN(what) PROTECT( \
    auto it = what; \
    if (DyErr_Occurred()) \
        return nullptr; \
    else \
        return make_return<Return>(it); \
    )

#define PROTECT_VOID(what) PROTECT(what;) \
    if (DyErr_Occurred()) \
        return nullptr; \
    else \
        return Dy_Retain(Dy_None);

    // Void, no self
    MAKE(vs)
    {
        PROTECT_VOID((*fn)(get_arg<I>(args)...))
    }

    MAKE(s)
    {
        PROTECT_RETURN((*fn)(get_arg<I>(args)...))
    }

    MAKE(vo)
    {
        if (!self)
            self = Dy_Undefined;
        PROTECT_VOID((*fn)(self, get_arg_o<I>(args)...))
    }

    MAKE(o)
    {
        if (!self)
            self = Dy_Undefined;
        PROTECT_RETURN((*fn)(self, get_arg_o<I>(args)...))
    }
#undef MAKE
#undef PROTECT_RETURN
#undef PROTECT_VOID
#undef PROTECT
};

template <typename... A>
DyObject *mkfunction(void(*fn)(A...))
{
    using fx = Callable::fnx<void, A...>;
    return fx::createvs(new typename fx::function(fn), std::index_sequence_for<A...>{});
}

template <typename R, typename... A>
DyObject *mkfunction(R(*fn)(A...))
{
    using fx = Callable::fnx<R, A...>;
    return fx::creates(new typename fx::function(fn), std::index_sequence_for<A...>{});
}

template <typename... A>
DyObject *mkmethod(void(*fn)(const Object &, A...))
{
    using fx = Callable::fnx<void, const Object &, A...>;
    return fx::createvo(new typename fx::function(fn), std::index_sequence_for<A...>{});
}

template <typename R, typename... A>
DyObject *mkmethod(R(*fn)(const Object &, A...))
{
    using fx = Callable::fnx<R, const Object &, A...>;
    return fx::createo(new typename fx::function(fn), std::index_sequence_for<A...>{});
}

// lambda
// http://stackoverflow.com/questions/11893141/inferring-the-call-signature-of-a-lambda-or-arbitrary-callable-for-make-functio
template<typename T> struct remove_class { };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...)> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) volatile> { using type = R(A...); };
template<typename C, typename R, typename... A>
struct remove_class<R(C::*)(A...) const volatile> { using type = R(A...); };

template<typename T>
struct get_signature_impl {
    using type = typename remove_class<
    decltype(&std::remove_reference<T>::type::operator())>::type;
};
template<typename R, typename... A>
struct get_signature_impl<R(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(&)(A...)> { using type = R(A...); };
template<typename R, typename... A>
struct get_signature_impl<R(*)(A...)> { using type = R(A...); };
template<typename T>
using get_signature = typename get_signature_impl<T>::type;

// TODO: use the above to support lambdas (overload mkfunction and mkmethod)

/// @endcond

} // namespace Callable

/**
 * @brief Create a callable libdy userdata
 * @ingroup Callables
 * @param fn The callable C++ thing
 * @return A callable Userdata
 */
template <typename T>
Userdata function(T fn)
{
    return {Callable::mkfunction(fn), true};
}

/**
 * @brief Create a callable libdy userdata
 * @ingroup Callables
 * @param fn The callable C++ thing
 * @return  A callable Userdata
 * @sa function()
 * @note The C++ callable will take an Object as first argument, corresponding
 *       to the container the callable was retrieved from.
 * @note Unfortunately, there's no way to make sure methods are only called
 *       as methods (i.e. with the self argument) and not also as plain functions
 *       that are preemtively retrieved from their container. In such cases,
 *       the first argument will be Undefined
 */
template <typename T>
Userdata method(T fn)
{
    return {Callable::mkmethod(fn), true};
}

}
