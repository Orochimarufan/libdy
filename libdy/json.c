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

#include "json.h"

#include "dy.h"
#include "json_token.h"
#include "buildstring.h"
#include "linalloc.h"
#include "host_p.h"
#include "exceptions.h"
#include "dy_p.h"

#include <assert.h>


typedef DyObject *(*token_handler_t)(dyj_token_t*, DyJson_NextChunkFn_t, void*);

static bool next_token(dyj_token_t*, DyJson_NextChunkFn_t, void*);
static bool check_token(dyj_token_t*, dyj_token_type expected, dyj_token_type or);
inline static DyObject *this_or(dyj_token_t*, DyJson_NextChunkFn_t, void*, dyj_token_type or);


#define HANDLER(name) \
    static DyObject *handle_ ## name(dyj_token_t *token, \
                          DyJson_NextChunkFn_t chunk, \
                          void *chunk_data)

HANDLER(OBJECT)
{
    DyObject *dict = DyDict_New();
    if (!dict)
        return_null;

    while (true)
    {
        if (!next_token(token, chunk, chunk_data))
            goto cleanup;

        if (token->type == TOKEN_BRACE_CLOSE)
            break;

        DyObject *key = this_or(token, chunk, chunk_data, TOKEN_BRACE_CLOSE);
        if (!key)
            goto cleanup;

        if (!next_token(token, chunk, chunk_data)
         || !check_token(token, TOKEN_COLON, 0))
            goto cleanup;

        DyObject *value = DyJson_NextEx(token, chunk, chunk_data);
        if (!value)
        {
            Dy_Release(key);
            goto cleanup;
        }

        bool res = Dy_SetItem(dict, key, value);
        Dy_Release(key);
        Dy_Release(value);

        if (!res)
            goto cleanup;

        if (!next_token(token, chunk, chunk_data))
            goto cleanup;

        if (token->type == TOKEN_BRACE_CLOSE)
            break;
        else if (!check_token(token, TOKEN_COMMA, TOKEN_BRACE_CLOSE))
            goto cleanup;
    }

    return dict;

cleanup:
    Dy_Release(dict);
    return_null;
}

HANDLER(ARRAY)
{
    DyObject *list = DyList_New();
    if (!list)
        return_null;

    while (true)
    {
        if (!next_token(token, chunk, chunk_data))
            goto cleanup;

        if (token->type == TOKEN_BRACKET_CLOSE)
            break;

        DyObject *item = this_or(token, chunk, chunk_data, TOKEN_BRACKET_CLOSE);
        if (!item)
            goto cleanup;

        bool res = DyList_Append(list, item);
        Dy_Release(item);

        if (!res)
            goto cleanup;

        if (!next_token(token, chunk, chunk_data))
            goto cleanup;

        if (token->type == TOKEN_BRACKET_CLOSE)
            break;
        else if (!check_token(token, TOKEN_COMMA, TOKEN_BRACKET_CLOSE))
            goto cleanup;
    }

    return list;

cleanup:
    Dy_Release(list);
    return_null;
}

HANDLER(STRING)
{
    dyj_string_token_t strtok;
    dy_buildstring_t *bs = NULL;
    dy_linalloc_t *mem = dy_linalloc_new(6 * 32, dy_malloc);
    if (!mem)
    {
        DyErr_SetMemoryError();
        return_null;
    }

    dyj_init_string(&strtok, token);
    dyj_next_string(&strtok);


    while (dyj_next_string(&strtok))
    {
        if (strtok.type == DYJ_STRTOK_QUOTE)
            break;

        else if (strtok.type == DYJ_STRTOK_ESCAPE)
        {
            // FIXME: this is dirty... fix it
            uint8_t *buf = (uint8_t*) dy_linalloc_malloc(mem, 6);
            if (!buf)
            {
                strtok.type = DYJ_STRTOK_INVALID;
                strtok.error = "Too many escape sequences";
                break;
            }
            size_t len = dyj_unicode_utf8(strtok.escape, buf);
            dy_buildstring_more(&bs, (char*)buf, len);
        }
        else
            dy_buildstring_more(&bs, strtok.begin, strtok.end - strtok.begin);
    }

    if (strtok.type == DYJ_STRTOK_INVALID)
    {
        DyErr_Format(DY_ERRID_JSON_PARSE_STRING,
                     "%s (at line %d, column %d)",
                     strtok.error, token->location.line,
                     token->location.column + strtok.error_offset);
        dy_buildstring_free(bs);
        dy_free(mem);
        return_null;
    }
    else
    {
        DyObject *result = dy_buildstring_build(bs);
        dy_buildstring_free(bs);
        dy_free(mem);
        return result;
    }
}

HANDLER(INT)
{
    return DyLong_New(token->int_value);
}

HANDLER(FLOAT)
{
    return DyFloat_New(token->float_value);
}

HANDLER(NULL)
{
    return Dy_Retain(Dy_None);
}

HANDLER(TRUE)
{
    return Dy_Retain(Dy_True);
}

HANDLER(FALSE)
{
    return Dy_Retain(Dy_False);
}

HANDLER(SPACE)
{
    return DyJson_NextEx(token, chunk, chunk_data);
}

#undef HANDLER


// Maps Json tokens to handlers
// See dyj.h
token_handler_t handlers[] = {
    NULL, // Invalid
    NULL, // EOF
    handle_OBJECT, // BRACE_OPEN
    NULL, // BRACE_CLOSE
    handle_ARRAY, // BRACKET_OPEN
    NULL, // BRACKET_CLOSE
    NULL, // COLON
    NULL, // COMMA,
    handle_STRING, // STRING
    handle_INT, // INT
    handle_FLOAT, // FLOAT
    handle_NULL, // NULL
    handle_TRUE, // TRUE
    handle_FALSE, // FALSE
    handle_SPACE, // SPACE; ignored, see next_token()
};


inline DyObject *DyJson_ThisEx(dyj_token_t *token, DyJson_NextChunkFn_t chunk, void *chunkud)
{
    token_handler_t handle = handlers[token->type];
    if (!handle)
    {
        DyErr_Format(DY_ERRID_JSON_PARSE,
                     "Expected value, got %s at line %d, column %d",
                     dyj_token_names[token->type],
                     token->location.line, token->location.column);
        return_null;
    }

    return handle(token, chunk, chunkud);
}

inline static DyObject *this_or(dyj_token_t *token, DyJson_NextChunkFn_t chunk, void *chunkud, dyj_token_type or)
{
    token_handler_t handle = handlers[token->type];
    if (!handle)
    {
        DyErr_Format(DY_ERRID_JSON_PARSE,
                     "Expected value or %s, got %s at line %d, column %d",
                     dyj_token_names[or], dyj_token_names[token->type],
                     token->location.line, token->location.column);
        return_null;
    }

    return handle(token, chunk, chunkud);
}

DyObject *DyJson_ThisEx(dyj_token_t*, DyJson_NextChunkFn_t, void*);

DyObject *DyJson_NextEx(dyj_token_t *token, DyJson_NextChunkFn_t chunk, void *chunkud)
{
    if (!next_token(token, chunk, chunkud))
        return_null;

    return DyJson_ThisEx(token, chunk, chunkud);
}

static bool next_token(dyj_token_t *token, DyJson_NextChunkFn_t chunk, void *chunk_data)
{
    token->type = TOKEN_SPACE;

    while (token->type == TOKEN_SPACE)
    {
        if (dyj_next_token(token))
            continue;

        if (token->error == dyj_chunk_end && chunk)
        {
            chunk(token, chunk_data);

            if (dyj_next_token(token))
                continue;
        }

        DyErr_Format(DY_ERRID_JSON_TOKEN,
                     "%s (at line %d, column %d)",
                     token->error, token->error_location.line, token->error_location.column);

        return_error(false);
    }

    return true;
}

static bool check_token(dyj_token_t *token, dyj_token_type expect, dyj_token_type or)
{
    if (token->type != expect)
    {
        if (!or)
            DyErr_Format(DY_ERRID_JSON_PARSE,
                         "Expected %s, got %s at line %d, column %d",
                         dyj_token_names[expect], dyj_token_names[token->type],
                         token->location.line, token->location.column);
        else
            DyErr_Format(DY_ERRID_JSON_PARSE,
                         "Expected %s or %s, got %s at line %d, column %d",
                         dyj_token_names[expect], dyj_token_names[or], dyj_token_names[token->type],
                         token->location.line, token->location.column);

        return_error(false);
    }
    return true;
}


DyObject *DyJson_Parse(const char *json)
{
    dyj_token_t tok;

    dyj_init_token(&tok, json);
    return DyJson_NextEx(&tok, NULL, NULL);
}
