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

#include "dypp.h"
#include "dypp_conv.h"

#include <libdy/userdata.h>

#include <functional>
#include <tuple>
#include <utility>
#include <type_traits>

namespace Dy {
namespace Callable {

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
        DyObject *a = Dy_GetItemLong(args, i);
        if (!a)
            throw_exception();
        return make_argument<NthArgType<i>>(Dy_Pass(a));
    }

    template <std::size_t i>
    static inline NthArgType<i> get_arg_o(DyObject *args)
    {
        DyObject *a = Dy_GetItemLong(args, i);
        if (!a)
            throw_exception();
        return make_argument<NthArgType<i + 1>>(Dy_Pass(a));
    }

    /*template <class X=Return, typename std::enable_if<!std::is_void<X>::value, int>::type = 0, std::size_t... I>
    static DyObject *simple_call(DyObject *, function *fn, DyObject *args)
    {
        DyObject *res = (*fn)(get_arg<I>(args)...);
        if (!res)
            return NULL;
        return Dy::conv::from_value_or_ref(res); // FIXME: don't Dy_Retain DyObject*s
    }*/

    // Helper macros
#define MAKE(n) \
    template <std::size_t... I> \
    static inline DyObject *create ##n (function *fn, std::index_sequence<I...>) \
    { \
        return make_udata(reinterpret_cast<DyUser_Callback>(&call ##n<I...>), fn); \
    } \
    template <std::size_t... I> \
    static DyObject *call ##n(DyObject *self, function *fn, DyObject *args)

    // Void, no self
    MAKE(vs)
    {
        (*fn)(get_arg<I>(args)...);
        return Dy_Retain(Dy_None);
    }

    MAKE(s)
    {
        Return result = (*fn)(get_arg<I>(args)...);
        if (DyErr_Occurred())
            return nullptr;
        else
            return make_return<Return>(result);
    }

    MAKE(vo)
    {
        if (!self)
            self = Dy_Undefined;
        (*fn)(self, get_arg_o<I>(args)...);
        return Dy_Retain(Dy_None);
    }

    MAKE(o)
    {
        if (!self)
            self = Dy_Undefined;
        Return result = (*fn)(self, get_arg_o<I>(args)...);
        if (DyErr_Occurred())
            return nullptr;
        else
            return make_return<Return>(result);
    }
#undef MAKE
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


} // namespace Callable

template <typename T>
Object function(T fn)
{
    return Object(Callable::mkfunction(fn), true);
}

template <typename T>
Object method(T fn)
{
    return Object(Callable::mkmethod(fn), true);
}

}
