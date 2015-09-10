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

#include "util.h"
#include "dy++.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QStringList>
#include <QtCore/QHash>
#include <QtCore/QVariant>

namespace Dy {
namespace conv {

template <>
struct convert<QString> {
    static inline DyObject *from_value(const QString &str)
    {
        QByteArray utf8 = str.toUtf8();
        return DyString_FromStringAndSize(utf8.constData(), utf8.size());
    }
    static inline QString to_value(DyObject *object)
    {
        if (!DyString_Check(object))
            format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Could not convert Dy::%s to QString", Dy_GetTypeName(Dy_Type(object)));
        return QString::fromUtf8(QByteArray(DyString_AsString(object), Dy_Length(object)));
    }
};

template <typename Seq>
struct convert_generic_sequence {
    static inline DyObject *from_value(const Seq &l)
    {
        Dy::util::safe_dy_ptr lst = DyList_NewEx(l.size());
        if(!lst)
            throw_exception();

        for (const typename Seq::value_type &i : l)
        {
            Dy::util::safe_dy_ptr obj = convert<typename Seq::value_type>::from_value(i);
            if (!obj)
                throw_exception();

            if (!DyList_Append(lst, obj))
                throw_exception();
        }

        return lst.pass();
    }
    static inline Seq to_value(DyObject *obj)
    {
        if (!DyList_Check(obj))
            format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Could not convert Dy::%s to QList", Dy_GetTypeName(Dy_Type(obj)));

        Seq result;
        result.reserve(Dy_Length(obj));

        for (size_t i=0; i<Dy_Length(obj); ++i)
            result.push_back(convert<typename Seq::value_type>::to_value(Dy_GetItemLong(obj, i)));

        return result;
    }
};

template <typename T>
struct convert<QList<T>> {
    static inline DyObject *from_value(const QList<T> &val)
    {
        return convert_generic_sequence<QList<T>>::from_value(val);
    }
    static inline QList<T> to_value(DyObject *obj)
    {
        return convert_generic_sequence<QList<T>>::to_value(obj);
    }
};

template <>
struct convert<QStringList> {
    static inline DyObject *from_value(const QStringList &val)
    {
        return convert_generic_sequence<QStringList>::from_value(val);
    }
    static inline QStringList to_value(DyObject *obj)
    {
        return convert_generic_sequence<QStringList>::to_value(obj);
    }
};

template <typename Mapping>
struct convert_generic_mapping {
    static inline DyObject *from_value(const Mapping &m)
    {
        Dy::util::safe_dy_ptr dct = DyDict_New();
        if (!dct)
            throw_exception();

        typename Mapping::const_iterator it;
        for (it=m.constBegin(); it != m.constEnd(); ++it)
        {
            Dy::util::safe_dy_ptr k = convert<typename Mapping::key_type>::from_value(it.key());
            if (!k)
                throw_exception();

            Dy::util::safe_dy_ptr v = convert<typename Mapping::mapped_type>::from_value(it.value());
            if (!v)
                throw_exception();

            if (!Dy_SetItem(dct, k, v))
                throw_exception();
        }

        return dct.pass();
    }
    static inline Mapping to_value(DyObject *obj)
    {
        if (!DyDict_Check(obj))
            format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Cannot convert Dy::%s to Mapping", Dy_GetTypeName(Dy_Type(obj)));

        Dy::util::safe_ptr<DyDict_IterPair*, DyDict_IterFree> iter = DyDict_Iter(obj);

        if (!iter)
            throw_exception();
        if (!*iter)
            return Mapping();

        Mapping result;
        do
        {
            result.insert(convert<typename Mapping::key_type>::to_value((*iter)->key),
                convert<typename Mapping::mapped_type>::to_value((*iter)->value));
        }
        while (DyDict_IterNext(iter));

        return result;
    }
};

template <typename Key, typename T>
struct convert<QHash<Key, T>> {
    static inline DyObject *from_value(const QHash<Key, T> &val)
    {
        return convert_generic_mapping<QHash<Key, T>>::from_value(val);
    }
    static inline QHash<Key, T> to_value(DyObject *obj)
    {
        return convert_generic_mapping<QHash<Key, T>>::to_value(obj);
    }
};

template <>
struct convert<QVariant> {
    static inline DyObject *from_value(const QVariant &v)
    {
        if (v.isNull())
            return Dy_Retain(Dy_None);

        switch((QMetaType::Type)v.type())
        {
            case QMetaType::Bool:
                if (v.toBool())
                    return Dy_Retain(Dy_True);
                else
                    return Dy_Retain(Dy_False);
            case QMetaType::Int:
            case QMetaType::UInt:
            case QMetaType::Long:
            case QMetaType::LongLong:
            case QMetaType::ULong:
            case QMetaType::UShort:
            case QMetaType::Char:
            case QMetaType::SChar:
            case QMetaType::UChar:
                return DyLong_New(v.toLongLong());
            case QMetaType::Double:
            case QMetaType::Float:
                return DyFloat_New(v.toDouble());
            case QMetaType::QString:
            case QMetaType::QUrl:
                return convert<QString>::from_value(v.toString());
            case QMetaType::QVariantList:
                return convert<QVariantList>::from_value(v.value<QVariantList>());
            case QMetaType::QVariantHash:
                return convert<QVariantHash>::from_value(v.value<QVariantHash>());
            case QMetaType::QStringList:
                return convert<QStringList>::from_value(v.value<QStringList>());
            default:
                format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Cannot convert QVariant of type %s to libdy object", QMetaType::typeName(v.type()));
        }
    }
    static inline QVariant to_value(DyObject *obj)
    {
        switch(Dy_Type(obj))
        {
            case DY_NONE:
                return QVariant();
            case DY_BOOL:
                return QVariant(obj == Dy_True);
            case DY_LONG:
                return QVariant((long long)DyLong_Get(obj));
            case DY_FLOAT:
                return QVariant(DyFloat_Get(obj));
            case DY_STRING:
                return QVariant(convert<QString>::to_value(obj));
            case DY_DICT:
                return QVariant::fromValue(convert<QHash<QString, QVariant>>::to_value(obj));
            case DY_LIST:
                return QVariant::fromValue(convert<QList<QVariant>>::to_value(obj));
            default:
                format_exception(LIBDY_ERROR_CXX_TYPE_ERROR, "Cannot convert Dy::%s to QVariant", Dy_GetTypeName(Dy_Type(obj)));
        }
    }
};

}
}
