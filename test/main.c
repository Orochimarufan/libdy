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

#include "libdy/dy.h"
#include "libdy/dy_error.h"
#include "libdy/dy_callable.h"

#include <stdio.h>


void Dy_Print(DyObject *o)
{
    if (!o)
    {
        DyErr_Set(DY_ERRID_TYPE_ERROR, "Dy_Print: (null)");
        return;
    }

    DyObject *s = Dy_Str(o);
    printf("%s\n", DyString_AsString(s));
    Dy_Release(s);
}

DyObject *hello()
{
    puts("hello world!");
    return Dy_Retain(Dy_None);
}

int main(void)
{
    DyObject *str = DyString_FromString("Hello, World");
    DyObject *obj = DyDict_New();
    DyObject *obj2 = DyDict_NewWithParent(obj);
    DyObject *fn = DyCall_CreateNoArg(&hello);
    
    Dy_Release(Dy_Call0(fn));
    Dy_SetItemString(obj, "fn", fn);

    Dy_Print(str);

    Dy_SetItemString(obj, "hello", str);
    Dy_Print(Dy_GetItemString(obj, "hello"));
    Dy_Print(Dy_GetItemString(obj2, "hello"));

    Dy_SetItemString(obj, "Derp", Dy_Pass(DyString_FromString("HAHAHA")));
    Dy_Print(obj);
    
    Dy_SetItemString(obj, "Derp", NULL);
    Dy_Print(obj);

    DY_ERR_HANDLER
        DY_ERR_CATCH_ALL(e)
            puts(DyErr_Message(e));
    DY_ERR_HANDLER_END

    Dy_Release(str);
    Dy_Release(obj);
    Dy_Release(obj2);

    fflush(stdout);

    return 0;
}
