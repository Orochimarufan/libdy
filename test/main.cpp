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
#include <libdy/userdata.h>

void Dy_Print(const Dy::Object &o)
{
    puts(o.str());
}

DyObject *hello()
{
    puts("hello world!");
    return Dy_Retain(Dy_None);
}

DyObject *callLambda(void *lambda)
{
    ((std::function<void()>*)lambda)->operator()();
    return Dy_Retain(Dy_None);
}

int _main(void)
{
    Dy::Object str("Hello, World");
    Dy::Object obj(DyDict_New(), true);
    Dy::Object obj2(DyDict_NewWithParent(obj.get()), true);
    Dy::Object fn(DyCall_CreateNoArg(&hello), true);

    std::function<void()> lfn = [] () { puts("hello lambda!"); };
    Dy::Object la(DyCall_CreateNoArgWithData(&callLambda, &lfn), true);

    fn();
    la();

    Dy_Print(str);

    obj["hello"] = str;
    Dy_Print(obj["hello"]);
    Dy_Print(obj2["hello"]);

    obj["Derp"] = "HAHAHA";
    Dy_Print(obj);

    obj["Derp"].del();
    Dy_Print(obj);

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
        return 1;
    }
}