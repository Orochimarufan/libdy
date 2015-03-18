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

#include "dy_p.h"
#include "string_p.h"
#include "dict_p.h"
#include "list_p.h"
#include "callable_p.h"
#include "dy_error.h"
#include "dy_string.h"

#include <stdlib.h>
#include <memory.h>
//#include <stdatomic.h>
#include <stdio.h>
#include <assert.h>


void Dy_InitObject(DyObject *o, DyObject_Type t)
{
    o->refcnt = 1;
    o->type = t;
}

// Constants
DyObject _dy_undefined = { .type = DY_NONE, .refcnt = 1 };
DyObject *Dy_Undefined = &_dy_undefined;

bool      DyUndefined_Check(DyObject *obj);

DyObject _dy_none = { .type = DY_NONE, .refcnt = 1 };
DyObject *Dy_None = &_dy_none;

bool      DyNone_Check(DyObject *obj);

DyObject _dy_true = { .type = DY_BOOL, .refcnt = 1 };
DyObject *Dy_True = &_dy_true;

DyObject _dy_false = { .type = DY_BOOL, .refcnt = 1 };
DyObject *Dy_False = &_dy_false;

bool      DyBool_Check(DyObject *obj);
DyObject *DyBool_Get(bool value);

// Number
typedef struct _DyIntegral_Object {
    DyObject_HEAD;
    int64_t value;
} DyIntegral_Object;

bool DyLong_Check(DyObject *self);

DyObject *DyLong_New(int64_t value)
{
    DyObject *o = NEW(DyIntegral_Object);
    Dy_InitObject(o, DY_LONG);
    ((DyIntegral_Object*)o)->value = value;
    return o;
}

int64_t DyLong_Get(DyObject *self)
{
    if (!DyLong_Check(self))
    {
        DyErr_SetArgumentTypeError("DyIntegral_GetInt", 0, "Integral", Dy_GetTypeName(Dy_Type(self)));
        return 0;
    }
    return ((DyIntegral_Object*)self)->value;
}

typedef struct _DyFloating_Object {
    DyObject_HEAD;
    double value;
} DyFloating_Object;

bool DyFloat_Check(DyObject *self);

DyObject *DyFloat_New(double value)
{
    DyObject *o = NEW(DyFloating_Object);
    Dy_InitObject(o, DY_FLOAT);
    ((DyFloating_Object*)o)->value = value;
    return o;
}

double DyFloat_Get(DyObject *self)
{
    if (!DyFloat_Check(self))
    {
        DyErr_SetArgumentTypeError("DyFloating_GetDouble", 0, "Floating", Dy_GetTypeName(Dy_Type(self)));
        return 0;
    }
    return ((DyFloating_Object*)self)->value;
}

// Misc
DyObject *Dy_Retain(DyObject *self)
{
    ++self->refcnt;
    return self;
}

void Dy_Release(DyObject *self)
{
    if (!--self->refcnt)
        Dy_FreeObject(self);
}

DyObject *Dy_Pass(DyObject *self)
{
    --self->refcnt;
    return self;
}

void Dy_FreeObject(DyObject *o)
{
    static int level = 0;

    if (!level)
    {
        ++level;
        //DyObject *s = Dy_Repr(o);
        //printf("Free %s object at %p\n", Dy_GetTypeName(o->type), o);//DyString_AsString(s));
        //Dy_Release(s);
        --level;
    }

    if (o->type == DY_STRING)
        string_destroy((DyStringObject*) o);
    else if (o->type == DY_DICT)
        dict_destroy((DyDictObject*) o);
    else if (o->type == DY_LIST)
    	list_destroy((DyListObject *) o);

    free(o);
}


DyObject_Type Dy_Type(DyObject *o)
{
    return o->type;
}

bool Dy_HashEx(DyObject *self, Dy_hash_t *hash)
{
    switch(self->type)
    {
    case DY_STRING:
        *hash = string_hash((DyStringObject*)self);
        return true;
    case DY_LONG:
        *hash = ((DyIntegral_Object*)self)->value;
        return true;
    default:
        return false;
    }
}

Dy_hash_t Dy_Hash(DyObject *self)
{
    Dy_hash_t result;
    if (!Dy_HashEx(self, &result))
    {
    	DyErr_Set(DY_ERRID_NOT_HASHABLE, "Object is not hashable.");
    	return 0;
    }
    return result;
}

bool Dy_Equals(DyObject *a, DyObject *b)
{
    if (a == b)
        return true;
    if (a->type != b->type)
        return false;
    switch(a->type)
    {
    case DY_BOOL:
        assert(false && "This case should always be handled by a == b");
    case DY_LONG:
        return ((DyIntegral_Object*)a)->value == ((DyIntegral_Object*)b)->value;
    case DY_FLOAT:
        return ((DyFloating_Object*)a)->value == ((DyFloating_Object*)b)->value;
    case DY_STRING:
        return DyString_Equals(((DyStringObject *)a), ((DyStringObject *)b));
    default:
        return false; // FIXME: other types
    }
}

DyObject *Dy_Repr(DyObject *obj)
{
    static bool has_repr = false;
    static DyObject *none_repr;
    static DyObject *true_repr;
    static DyObject *false_repr;

    if (!has_repr)
    {
        has_repr = true;
        none_repr = DyString_InternStringFromString("None");
        true_repr = DyString_InternStringFromString("True");
        false_repr = DyString_InternStringFromString("False");
    }

    char buf[256];

    switch(obj->type)
    {
    case DY_NONE:
        return Dy_Retain(none_repr);
    case DY_BOOL:
        return Dy_Retain(obj == Dy_True ? true_repr : false_repr);
    case DY_LONG:
        snprintf(buf, 255, "%li", ((DyIntegral_Object*)obj)->value);
        return DyString_FromString(buf);
    case DY_FLOAT:
        snprintf(buf, 255, "%f", ((DyFloating_Object*)obj)->value);
        return DyString_FromString(buf);
    case DY_STRING:
        return string_repr((DyStringObject *)obj);
    case DY_DICT:
        return dict_repr((DyDictObject *)obj);
    case DY_LIST:
    	return list_repr((DyListObject *)obj);
    case DY_CALLABLE:
    	snprintf(buf, 255, "<Callable at 0x%p (%x)>", ((DyCallableObject*)obj)->function, ((DyCallableObject*)obj)->flags);
    	return DyString_FromString(buf);
    case DY_EXCEPTION:
        snprintf(buf, 255, "<Exception %s: %s>", DyErr_ErrId(obj), DyErr_Message(obj));
        return DyString_FromString(buf);
    default:
        return DyString_InternStringFromString("<<NotImplemented>>");
    }
}

DyObject *Dy_Str(DyObject *obj)
{
    switch(obj->type)
    {
    case DY_STRING:
        return Dy_Retain(obj);
    default:
        return Dy_Repr(obj);
    }
}

const char *Dy_AsRepr(DyObject *self)
{
    static char buffer[1025];
    
    DyObject *s = Dy_Repr(self);
    strncpy(buffer, DyString_AsString(s), 1024);
    Dy_Release(s);
    
    buffer[1024] = 0;
    
    return buffer;
}

// TypeErrors
inline static void TE__notsubscriptable(DyObject *o)
{
    const char *tn = Dy_GetTypeName(Dy_Type(o));
    char *txt = malloc(strlen(tn) + 30);
    sprintf(txt, "%s object is not subscriptable", tn);
    DyErr_Set(DY_ERRID_TYPE_ERROR, txt);
    free(txt);
}

inline static void TE__listindex(const char *tn)
{
    char *txt = malloc(strlen(tn) + 43);
    sprintf(txt, "Lists only support numerical indices, not %s", tn);
    DyErr_Set(DY_ERRID_TYPE_ERROR, txt);
    free(txt);
}

DyObject *Dy_GetItem(DyObject *self, DyObject *key)
{
    switch (self->type)
    {
    case DY_DICT:
    	return dict_getitem((DyDictObject *)self, key);
    case DY_LIST:
    	if (key->type != DY_LONG)
    	{
    		TE__listindex(Dy_GetTypeName(Dy_Type(key)));
    		return NULL;
    	}
    	return list_getitem((DyListObject *)self, DyLong_Get(key));
    default:
    	TE__notsubscriptable(self);
    	return NULL;
    }
}

DyObject *Dy_GetItemString(DyObject *self, const char *key)
{
    switch (self->type)
    {
    case DY_DICT:
    {
    	DyObject *k = DyString_InternStringFromString(key);
    	DyObject *r = dict_getitem((DyDictObject *)self, k);
    	Dy_Release(k);
    	return r;
    }
    case DY_LIST:
    	TE__listindex(Dy_GetTypeName(DY_STRING));
    	return NULL;
    default:
    	TE__notsubscriptable(self);
    	return NULL;
    }
}

DyObject *Dy_GetItemLong(DyObject *self, long key)
{
    switch (self->type)
    {
    case DY_DICT:
    {
    	DyObject *k = DyLong_New(key);
    	DyObject *r = dict_getitem((DyDictObject *)self, k);
    	Dy_Release(k);
    	return r;
    }
    case DY_LIST:
    	return list_getitem((DyListObject *)self, key);
    default:
    	TE__notsubscriptable(self);
    	return NULL;
    }
}

DyObject *Dy_GetItemU(DyObject *self, DyObject *key)
{
    switch (self->type)
    {
    case DY_DICT:
    	return dict_getitemu((DyDictObject *)self, key);
    case DY_LIST:
    	if (key->type != DY_LONG)
    	{
    		TE__listindex(Dy_GetTypeName(Dy_Type(key)));
    		return NULL;
    	}
    	return list_getitemu((DyListObject *)self, DyLong_Get(key));
    default:
    	TE__notsubscriptable(self);
    	return NULL;
    }
}

bool Dy_SetItem(DyObject *self, DyObject *key, DyObject *value)
{
    switch (self->type)
    {
    case DY_DICT:
    	return dict_setitem((DyDictObject *)self, key, value);
    case DY_LIST:
    	if (key->type != DY_LONG)
    	{
    		TE__listindex(Dy_GetTypeName(Dy_Type(key)));
    		return false;
    	}
    	return list_setitem((DyListObject *) self, DyLong_Get(key), value);
    default:
    	TE__notsubscriptable(self);
    	return false;
    }
}

bool Dy_SetItemString(DyObject *self, const char *key, DyObject *value)
{
    switch (self->type)
    {
    case DY_DICT:
    {
    	DyObject *k = DyString_InternStringFromString(key);
    	bool r = dict_setitem((DyDictObject *)self, k, value);
    	Dy_Release(k);
    	return r;
    }
    case DY_LIST:
    	TE__listindex(key);
    	return false;
    default:
    	TE__notsubscriptable(self);
    	return false;
    }
}

bool Dy_SetItemLong(DyObject *self, long key, DyObject *value)
{
    switch (self->type)
    {
    	case DY_DICT:
    	{
    		DyObject *k = DyLong_New(key);
    		bool r = dict_setitem((DyDictObject *)self, k, value);
    		Dy_Release(k);
    		return r;
    	}
    	case DY_LIST:
    		return list_setitem((DyListObject *)self, key, value);
    	default:
    		TE__notsubscriptable(self);
    		return false;
    }
}

// Length
size_t Dy_Length(DyObject *self)
{
    switch (self->type)
    {
    case DY_LIST:
    	return ((DyListObject *)self)->size;
    case DY_STRING:
    	return ((DyStringObject *)self)->size;
    default:
    	DyErr_SetArgumentTypeError("Dy_Length", 0, "list or string", Dy_GetTypeName(self->type));
    	return 0;
    }
}