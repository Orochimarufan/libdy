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

#include "test_support.hpp"

#include <libdy++/dy++.h>
#include <libdy++/dy++conv.h>
#include <libdy++/callable.h>

#include <cstdio>
#include <iostream>
#include <functional>
#include <cassert>

void Dy_Print(const Dy::Object &o)
{
    puts(o.str());
}

void hello()
{
    puts("hello world!");
}

void test_method(const Dy::Object &self)
{
    Dy_Print(self);
}

const char *rtest()
{
    return "hello world";
}

void test_exceptions()
{
    Dy::throw_exception("test.CXXTestException", "Thrown from userdata function");
}

static int ival = 3;

int _main(void)
{
    Dy::String str("Hello, World");

    puts(str);

    Dy::Dict dict{
        {"hello", Dy::function(::hello)},
        {"method", Dy::method(::test_method)},
        {"return", Dy::function(::rtest)},
        {"throw", Dy::function(::test_exceptions)},
        {"text", "Yo!"},
    };

    Dy::Dict dict2({
        {1, Dy::List{2, 3, 4, 5, "and more"}},
        {"print", Dy::function(::Dy_Print)},
    }, dict);

    Dy_Print(dict);
    Dy_Print(dict2);

    Dy_Print(dict2["text"]);
    dict2["stuff"] = 4;
    Dy_Print(dict2);

    puts("--------------------------------");

    dict2["hello"]();
    dict["method"]();
    dict2["method"]();
    dict2["print"](dict["return"]());

    puts("--------------------------------");

    puts("+++ Dict iter +++");
    for (auto it : dict)
        printf("%s = %s\n", it.key().str().c_str(), it.value().str().c_str());

    puts("+++ List iter +++");
    for (auto it : (Dy::List)dict2[1])
        Dy_Print(it);

    puts("--------------------------------");
    pcall(dict["throw"]);

    Dy::Userdata ud(&ival, "Ival pointer");
    Dy_Print(ud);
    Dy_Print(ud.data() == &ival);

    Dy::String s2 = std::move(str);
    assert(str == Dy::Undefined);

    // This is a case extracted from real-life that tests implicit conversions between Object and the subclasses:
    // The important part is that the conversion constructor on the subclasses must be preferred
    // over the template conversion operator on Object
    Dy::Dict stuff{{"hey", Dy::List{}}};
    Dy::List heys;
    heys.extend(stuff["hey"]); // This line will not compile if the preference is messed up.

    return 0;
}

int main(void)
{
    return pcall(_main) ? 1 : 0;
}
