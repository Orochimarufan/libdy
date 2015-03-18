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

#include "callable_p.h"
#include "dy.h"
#include "dy_error.h"

#include <assert.h>


// Inlines
bool DyCallable_Check(DyObject *);

// Create
DyObject *DyCall_CreateNoArg(DyNoArgFn fn)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_NOARG << 1) | DCD_NODATA;
    co->function = fn;
    return (DyObject*)co;
}

DyObject *DyCall_CreateNoArgWithData(DyNoArgFnWithData fn, void *data)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_NOARG << 1) | DCD_HASDATA;
    co->function = fn;
    co->data = data;
    return (DyObject*)co;
}

DyObject *DyCall_CreateSimple(DySimpleFn fn)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_SIMPLE << 1) | DCD_NODATA;
    co->function = fn;
    return (DyObject*)co;
}

DyObject *DyCall_CreateSimpleWithData(DySimpleFnWithData fn, void *data)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_SIMPLE << 1) | DCD_HASDATA;
    co->function = fn;
    co->data = data;
    return (DyObject*)co;
}

DyObject *DyCall_Create(DyArgListFn fn)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_ARGLIST << 1) | DCD_NODATA;
    co->function = fn;
    return (DyObject*)co;
}

DyObject *DyCall_CreateWithData(DyArgListFnWithData fn, void *data)
{
    DyCallableObject *co = NEW(DyCallableObject);
    Dy_InitObject((DyObject*)co, DY_CALLABLE);
    co->flags = (DCT_ARGLIST << 1) | DCD_HASDATA;
    co->function = fn;
    co->data = data;
    return (DyObject*)co;
}

// Check
bool DyCallable_Check(DyObject *self)
{
    return self->type == DY_CALLABLE;
}

// Call
DyObject *Dy_Call(DyObject *self, DyObject *arglist)
{
    if (DyErr_CheckArg("Dy_Call", 0, DY_CALLABLE, self) || DyErr_CheckArg("Dy_Call", 1, DY_LIST, arglist))
    	return NULL;
    
    DyCallableObject *co = (DyCallableObject *)self;
    
    switch(co->flags >> 1) // The type is all but the last bit of the flags.
    {
    case DCT_NOARG:
    	if (Dy_Length(arglist) != 0)
    	{
    		DyErr_Format(DY_ERRID_ARGUMENT_COUNT, "Callable takes no arguments, %i arguments given", Dy_Length(arglist));
    		return NULL;
    	}
    	
    	if (co->flags & DCD_HASDATA)
    		return ((DyNoArgFnWithData)co->function)(co->data);
    	else
    		return ((DyNoArgFn)co->function)();
    case DCT_SIMPLE:
    	if (Dy_Length(arglist) != 1)
    	{
    		DyErr_Format(DY_ERRID_ARGUMENT_COUNT, "Callable takes 1 argument, %i arguments given", Dy_Length(arglist));
    		return NULL;
    	}
    	
    	DyObject *arg = Dy_GetItemLong(arglist, 0);
    	
    	if (co->flags & DCD_HASDATA)
    		return ((DySimpleFnWithData)co->function)(arg, co->data);
    	else
    		return ((DySimpleFn)co->function)(arg);
    case DCT_ARGLIST:
    	if (co->flags & DCD_HASDATA)
    		return ((DyArgListFnWithData)co->function)(arglist, co->data);
    	else
    		return ((DyArgListFn)co->function)(arglist);
    default:
    	assert(false && "Should never happen!");
    }
}

DyObject *Dy_Call0(DyObject *self)
{
    if (DyErr_CheckArg("Dy_Call1", 0, DY_CALLABLE, self))
    	return NULL;
    
    DyCallableObject *co = (DyCallableObject *)self;
    
    if (co->flags >> 1 == DCT_NOARG)
    {
    	if (co->flags & DCD_HASDATA)
    		return ((DyNoArgFnWithData)co->function)(co->data);
    	else
    		return ((DyNoArgFn)co->function)();
    }
    else if (co->flags >> 1 == DCT_SIMPLE)
    {
    	DyErr_Set(DY_ERRID_ARGUMENT_COUNT, "Callable takes 1 argument, no arguments given");
    	return NULL;
    }
    else
    {
    	DyObject *arglist = DyList_New();
    	
    	DyObject *result = Dy_Call(self, arglist);
    	
    	Dy_Release(arglist);
    	
    	return result;
    }
}

DyObject *Dy_Call1(DyObject *self, DyObject *arg)
{
    if (DyErr_CheckArg("Dy_Call1", 0, DY_CALLABLE, self))
    	return NULL;
    
    DyCallableObject *co = (DyCallableObject *)self;
    
    if (co->flags >> 1 == DCT_NOARG)
    {
    	DyErr_Set(DY_ERRID_ARGUMENT_COUNT, "Callable takes no arguments, 1 argument given");
    	return NULL;
    }
    else if (co->flags >> 1 == DCT_SIMPLE)
    {
    	if (co->flags & DCD_HASDATA)
    		return ((DySimpleFnWithData)co->function)(arg, co->data);
    	else
    		return ((DySimpleFn)co->function)(arg);
    }
    else
    {
    	DyObject *arglist = DyList_New();
    	DyList_Append(arglist, arg);
    	
    	DyObject *result = Dy_Call(self, arglist);
    	
    	Dy_Release(arglist);
    	
    	return result;
    }
}

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
