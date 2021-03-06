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
#include <libdy++/dy++qt.h>

#include <cstdio>
#include <iostream>


int _main()
{
    QString hello("Hello Qt!");
    puts(Dy::Object(hello));

    Dy::Dict o{{"derp", "dum"}, {"dimp", 4}, {"dumb", false}};
    QVariantHash vh = o;
    puts(Dy::Object(vh).str());

    return 0;
}

int main(void)
{
    return pcall(_main) ? 1 : 0;
}
