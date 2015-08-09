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

#include <cstdio>
#include <iostream>
#include <functional>

#include <libdypp/dypp.h>
#include <libdypp/callable.h>
#include <libdy/userdata.h>

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

int _main(void)
{
    Dy::Object str("Hello, World");

    puts(str);

    Dy::Object dict = Dy::dict({
        {"hello", Dy::function(::hello)},
        {"method", Dy::method(::test_method)},
        {"return", Dy::function(::rtest)},
        {"text", "Yo!"},
    });

    Dy::Object dict2 = Dy::dict({
        {1, {2, 3, 4, 5, "and more"}},
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

    return 0;
}

int main(void)
{
    try {
        return _main();
    }
    catch (Dy::Exception &e)
    {
        std::cerr << e.errid() << ": " << e.what() << std::endl;
        DyObject *cause = e.cause();
        while(cause)
        {
            std::cerr << "Caused by: " << DyErr_ErrId(cause) << ": " << DyErr_Message(cause) << std::endl;
            cause = DyErr_Cause(cause);
        }
        e.clear();
        return 1;
    }
}
