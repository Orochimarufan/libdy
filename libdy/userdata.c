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

#include "userdata_p.h"
#include "dy.h"
#include "exceptions.h"

#include <assert.h>

// TODO: move the Dy_Call and Dy_Callable things out if non-userdata things can be callable

// Type Checking
bool DyUserdata_Check(DyObject *obj)
{
    return obj->type == DY_USERDATA;
}

bool DyCallable_Check(DyObject *obj)
{
    return obj->type == DY_USERDATA && ((DyUserdataObject*)obj)->call_fn;
}

// Simple
DyObject *DyUser_Create(void *data)
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject *)co, DY_USERDATA);
    co->data = data;
    return (DyObject *)co;
}

DyObject *DyUser_CreateNamed(void *data, const char *name)
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject *)co, DY_USERDATA);
    co->data = data;
    co->name = name;
    return (DyObject *)co;
}

void *DyUser_GetData(DyObject *ud)
{
    return ((DyUserdataObject*)ud)->data;
}

const char *DyUser_GetName(DyObject *ud)
{
    return ((DyUserdataObject*)ud)->name;
}

void userdata_destroy(DyObject *o)
{
    DyUserdataObject *co = (DyUserdataObject*) o;
    if (co->destructor_fn)
        co->destructor_fn(co->data);
}

bool DyUser_SetDestructor(DyObject *ud, DyUser_Destructor fn)
{
    if (DyErr_CheckArg("DyUser_SetDestructor", 0, DY_USERDATA, ud))
        return false;
    ((DyUserdataObject*)ud)->destructor_fn = fn;
}

// Create
DyObject *DyUser_CreateCallable(DyUser_Callback fn, void *data)
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject*)co, DY_USERDATA);
    co->flags = CBA_LIST;
    co->call_fn = fn;
    co->data = data;
    co->name = NULL;
    return (DyObject*)co;
}

DyObject *DyUser_CreateCallable0(DyUser_Callback0 fn, void *data)
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject*)co, DY_USERDATA);
    co->flags = CBA_0;
    co->call_fn = fn;
    co->data = data;
    co->name = NULL;
    return (DyObject*)co;
}

DyObject *DyUser_CreateCallable1(DyUser_Callback1 fn, void *data)
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject*)co, DY_USERDATA);
    co->flags = CBA_1;
    co->call_fn = fn;
    co->data = data;
    co->name = NULL;
    return (DyObject*)co;
}

static DyObject *_strip_args(DyObject *self, void *data)
{
    ((void(*)())data)();
    return Dy_Retain(Dy_None);
}

DyObject *DyUser_CreateCallback(void(*fn)())
{
    DyUserdataObject *co = NEW(DyUserdataObject);
    Dy_InitObject((DyObject*)co, DY_USERDATA);
    co->flags = CBA_0;
    co->call_fn = _strip_args;
    co->data = fn;
    co->name = NULL;
    return (DyObject*)co;
}

// Call
DyObject *DyCallable_Call(DyObject *callable, DyObject *self, DyObject *arglist)
{
    if (DyErr_CheckArg("Dy_Call", 0, DY_USERDATA, callable) || DyErr_CheckArg("Dy_Call", 2, DY_LIST, arglist))
        return_null;

    DyUserdataObject *co = (DyUserdataObject *)callable;
    if (!co->call_fn)
    {
        DyErr_SetArgumentTypeError("DyCallable_Call", 0, "Callable", "Simple Userdata");
        return_null;
    }

    switch(co->flags & 0x0F)
    {
    case CBA_0:
        if (Dy_Length(arglist) != 0)
        {
            DyErr_Format(DY_ERRID_ARGUMENT_COUNT, "Callable takes no arguments, %i arguments given", Dy_Length(arglist));
            return_null;
        }

        return ((DyUser_Callback0)co->call_fn)(self, co->data);

    case CBA_1:
        if (Dy_Length(arglist) != 1)
        {
            DyErr_Format(DY_ERRID_ARGUMENT_COUNT, "Callable takes 1 argument, %i arguments given", Dy_Length(arglist));
            return_null;
        }

        DyObject *arg = Dy_GetItemLong(arglist, 0);

        return ((DyUser_Callback1)co->call_fn)(self, co->data, arg);

    case CBA_LIST:
        return ((DyUser_Callback)co->call_fn)(self, co->data, arglist);

    default:
        assert(false && "Should never happen!");
    }
}

DyObject *DyCallable_Call0(DyObject *callable, DyObject *self)
{
    if (DyErr_CheckArg("DyCallable_Call0", 0, DY_USERDATA, callable))
        return_null;

    DyUserdataObject *co = (DyUserdataObject *)callable;
    if (!co->call_fn)
    {
        DyErr_SetArgumentTypeError("DyCallable_Call0", 0, "Callable", "Simple Userdata");
        return_null;
    }

    switch(co->flags & 0x0F)
    {
    case CBA_0:
        return ((DyUser_Callback0)co->call_fn)(self, co->data);

    case CBA_1:
        DyErr_Set(DY_ERRID_ARGUMENT_COUNT, "Callable takes 1 argument, no arguments given");
        return_null;

    case CBA_LIST:
    {
        DyObject *arglist = DyList_New();

        DyObject *result = ((DyUser_Callback)co->call_fn)(self, co->data, arglist);

        Dy_Release(arglist);

        return result;
    }

    default:
        assert(false && "Should never happen!");
    }
}

DyObject *DyCallable_Call1(DyObject *callable, DyObject *self, DyObject *arg)
{
    if (DyErr_CheckArg("DyCallable_Call1", 0, DY_USERDATA, callable))
        return_null;

    DyUserdataObject *co = (DyUserdataObject *)self;
    if (!co->call_fn)
    {
        DyErr_SetArgumentTypeError("DyCallable_Call1", 0, "Callable", "Simple Userdata");
        return_null;
    }

    switch(co->flags & 0x0F)
    {
    case CBA_0:
        DyErr_Set(DY_ERRID_ARGUMENT_COUNT, "Callable takes no arguments, 1 argument given");
        return_null;

    case CBA_1:
        return ((DyUser_Callback1)co->call_fn)(self, co->data, arg);

    case CBA_LIST:
    {
        DyObject *arglist = DyList_New();
        DyList_Append(arglist, arg);

        DyObject *result = ((DyUser_Callback)co->call_fn)(self, co->data, arglist);

        Dy_Release(arglist);

        return result;
    }

    default:
        assert(false && "Should never happen!");
    }
}

/*
DyObject *Dy_Call2(DyObject *self, DyObject *arg1, DyObject *arg2)
{
    DyObject *arglist = DyList_New();
    DyList_Append(arglist, arg1);
    DyList_Append(arglist, arg2);
    
    DyObject *result = Dy_Call(self, arglist);
        
    Dy_Release(arglist);
    
    return result;
}

DyObject *Dy_Call3(DyObject *self, DyObject *arg1, DyObject *arg2, DyObject *arg3)
{
    DyObject *arglist = DyList_New();
    DyList_Append(arglist, arg1);
    DyList_Append(arglist, arg2);
    DyList_Append(arglist, arg3);
    
    DyObject *result = Dy_Call(self, arglist);
        
    Dy_Release(arglist);
    
    return result;
}
*/

// Call Member
DyObject *Dy_Call(DyObject *self, DyObject *key, DyObject *arglist)
{
    DyObject *cb = Dy_GetItem(self, key);
    DyObject *res = DyCallable_Call(cb, self, arglist);
    Dy_Release(cb);
    return res;
}

DyObject *Dy_Call0(DyObject *self, DyObject *key)
{
    DyObject *cb = Dy_GetItem(self, key);
    DyObject *res = DyCallable_Call0(cb, self);
    Dy_Release(cb);
    return res;
}

DyObject *Dy_Call1(DyObject *self, DyObject *key, DyObject *arg)
{
    DyObject *cb = Dy_GetItem(self, key);
    DyObject *res = DyCallable_Call1(cb, self, arg);
    Dy_Release(cb);
    return res;
}

// Call Member by string
DyObject *Dy_CallString(DyObject *self, const char *key, DyObject *arglist)
{
    DyObject *cb = Dy_GetItemString(self, key);
    DyObject *res = DyCallable_Call(cb, self, arglist);
    Dy_Release(cb);
    return res;
}

DyObject *Dy_CallString0(DyObject *self, const char *key)
{
    DyObject *cb = Dy_GetItemString(self, key);
    DyObject *res = DyCallable_Call0(cb, self);
    Dy_Release(cb);
    return res;
}

DyObject *Dy_CallString1(DyObject *self, const char *key, DyObject *arg)
{
    DyObject *cb = Dy_GetItemString(self, key);
    DyObject *res = DyCallable_Call1(cb, self, arg);
    Dy_Release(cb);
    return res;
}
