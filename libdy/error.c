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

#include "exceptions.h"
#include "dy_p.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>


typedef struct _DyExceptionObject {
    DyObject_HEAD
    const char *errid;
    DyObject *cause;
    void *data;
    DyDataDestructor destruct_fn;
    char message[1];
} DyExceptionObject;

static _Thread_local DyObject *dy_error_state;

DyObject *DyErr_Occurred()
{
    return dy_error_state;
}

void DyErr_Clear()
{
    Dy_Release(dy_error_state);
    dy_error_state = NULL;
}

static inline DyObject *DyErr_NewExceptionX(const char *errid, size_t message_size, void *arg, DyDataDestructor fn)
{
    DyExceptionObject *self = dy_malloc(sizeof(DyExceptionObject) + message_size);
    Dy_InitObject((DyObject*)self, DY_EXCEPTION);
    self->errid = errid;
    self->cause = NULL;
    self->data = arg;
    self->destruct_fn = fn;
    return (DyObject *)self;
}

static inline DyObject *DyErr_NewException(const char *errid, const char *message, void *arg, DyDataDestructor fn)
{
    DyObject *self = DyErr_NewExceptionX(errid, strlen(message), arg, fn);
    strcpy(((DyExceptionObject *)self)->message, message);
    return self;
}

DyObject *DyErr_Set(const char *errid, const char *message)
{
    return DyErr_SetObject(Dy_Pass(DyErr_NewException(errid, message, NULL, NULL)));
}

DyObject *DyErr_Format(const char *errid, const char *format, ...)
{
    va_list va;
    va_start(va, format);
    size_t len = vsnprintf(NULL, 0, format, va);
    va_end(va);
    
    DyObject *e = DyErr_NewExceptionX(errid, len + 1, NULL, NULL);
    
    va_start(va, format);
    vsnprintf(((DyExceptionObject*)e)->message, len + 1, format, va);
    va_end(va);

    DyErr_SetObject(Dy_Pass(e));
    
    return e;
}

DyObject *DyErr_FormatV(const char *errid, const char *format, va_list args)
{
    va_list va;
    va_copy(va, args);
    size_t len = vsnprintf(NULL, 0, format, va);
    va_end(va);

    DyObject *e = DyErr_NewExceptionX(errid, len + 1, NULL, NULL);

    vsnprintf(((DyExceptionObject*)e)->message, len + 1, format, args);

    DyErr_SetObject(Dy_Pass(e));

    return e;
}

DyObject *DyErr_SetObject(DyObject *exception)
{
    DyErr_CheckArg("DyErr_SetObject", 1, DY_EXCEPTION, exception);
    ((DyExceptionObject *)exception)->cause = dy_error_state;
    dy_error_state = Dy_Retain(exception);
    return exception;
}

DyObject *DyErr_DiscardAndSetObject(DyObject *exception)
{
    DyErr_CheckArg("DyErr_DiscardAndSetObject", 1, DY_EXCEPTION, exception);
    if (dy_error_state)
        Dy_Release(dy_error_state);
    dy_error_state = Dy_Retain(exception);
    return exception;
}

bool DyErr_SetExceptionData(DyObject *exception, void *data, DyDataDestructor fn)
{
    if (DyErr_CheckArg("DyErr_SetExceptionData", 0, DY_EXCEPTION, exception))
        return false;
    ((DyExceptionObject*)exception)->data = data;
    ((DyExceptionObject*)exception)->destruct_fn = fn;
    return true;
}

void exception_destroy(DyObject *exc)
{
    if (((DyExceptionObject*)exc)->destruct_fn)
        ((DyExceptionObject*)exc)->destruct_fn(((DyExceptionObject*)exc)->data);
    if (((DyExceptionObject*)exc)->cause)
        Dy_Release(((DyExceptionObject*)exc)->cause);
}

// Argument checking
const char *DyObject_Type_Names[] = {
    "None",
    "Boolean",
    "Integral",
    "Floating",
    "String",
    "Object",
    "List",
    "Callable",
    "Exception"
};

DyObject *DyErr_SetArgumentTypeError(const char *fname, int arg_num, const char *expected, const char *got)
{
    size_t length = strlen(fname) + 42 + strlen(expected) + strlen(got);
    DyObject *self = DyErr_NewExceptionX(DY_ERRID_ARGUMENT_TYPE, length, NULL, NULL);
    snprintf(((DyExceptionObject *)self)->message, length, "%s(): Argument %i: expected %s but got %s object.", fname, arg_num, expected, got);
    return DyErr_SetObject(Dy_Pass(self));
}

DyObject *DyErr_CheckArg(const char *fname, int arg, DyObjectType expected, DyObject *got)
{
    if (got->type != expected)
        return DyErr_SetArgumentTypeError(fname, arg, DyObject_Type_Names[expected], DyObject_Type_Names[got->type]);
    return NULL;
}

const char *Dy_GetTypeName(DyObjectType t)
{
    return DyObject_Type_Names[t];
}

// Memory error
struct _DyExceptionObject __MemoryError = {
    .type = DY_EXCEPTION,
    .refcnt = 1,
    .errid = DY_ERRID_MEMORY_ERROR,
    .cause = NULL,
    .data = NULL,
    .message = "",
};

DyObject *DyErr_SetMemoryError()
{
    return DyErr_SetObject((DyObject *)&__MemoryError);
}

// Exception objects
bool DyErr_Filter(DyObject *exception, const char *errid)
{
    DyErr_CheckArg("DyErr_Filter", 0, DY_EXCEPTION, exception);
    const char *x = ((DyExceptionObject *)exception)->errid, *y = errid;
    for (; *x != 0 && *y != 0; ++x, ++y)
    	if (*x != *y)
    		return *(x+1) == '.';
    return *x == *y;
}

const char *DyErr_ErrId(DyObject *exception)
{
    DyErr_CheckArg("DyErr_ErrId", 0, DY_EXCEPTION, exception);
    return ((DyExceptionObject *)exception)->errid;
}

const char *DyErr_Message(DyObject *exception)
{
    DyErr_CheckArg("DyErr_Message", 0, DY_EXCEPTION, exception);
    return ((DyExceptionObject *)exception)->message;
}

DyObject *DyErr_Cause(DyObject *exception)
{
    DyErr_CheckArg("DyErr_Cause", 0, DY_EXCEPTION, exception);
    return ((DyExceptionObject *)exception)->cause;
}

void *DyErr_Data(DyObject *exception)
{
    DyErr_CheckArg("DyErr_Data", 0, DY_EXCEPTION, exception);
    return ((DyExceptionObject *)exception)->data;
}
