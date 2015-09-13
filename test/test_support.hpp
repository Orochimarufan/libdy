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

#include <libdy++/dy++.h>

#include <iostream>

inline void print_exception(Dy::Exception e)
{
    std::cerr << e.errid() << ": " << e.message() << std::endl;
    while(e.hasCause())
    {
        e = e.cause();
        std::cerr << ">> Caused by: " << e.errid() << ": " << e.message() << std::endl;
    }
}

template <typename Fn, typename... Args>
inline bool pcall(Fn f, Args... args)
{
    try {
        f(args...);
    }
    catch (Dy::Exception &e)
    {
        std::cerr << "pcall() caught Exception: ";
        print_exception(e);
        e.clear();
        return true;
    }
    return false;
}
