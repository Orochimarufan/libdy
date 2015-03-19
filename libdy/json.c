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

#include "json_p.h"
//#include "dy.h"

#include <math.h>
#include <string.h>


bool first_token(token *token, const char *data, const char *filename)
{
    return first_token_ex(token, data, filename, 0, 1, 1);
}

bool first_token_ex(token *token, const char *data, const char *filename, size_t offset, size_t line, size_t column)
{
    // Initialize token
    token->type = TOKEN_EOF;
    token->begin = data;
    token->end = data;
    token->end_location.filename = filename;
    token->end_location.offset = offset;
    token->end_location.line = line;
    token->end_location.column = column;
    token->error = NULL;

    // Find first token
    return next_token(token);
}


static bool token_space(token *token);
static bool token_char(token *token, token_type type);
static bool token_string(token *token);
static bool token_number(token *token);
static bool token_special(token *token, token_type type, const char *match);


bool next_token(token *token)
{
    // Next token
    memcpy(&token->location, &token->end_location, sizeof(token_location));
    token->begin = token->end;

    switch (*token->begin)
    {
        case 0x00:
            return token_char(token, TOKEN_EOF);
        case ' ':
        case '\n':
        case '\t':
            return token_space(token);
        case '{':
            return token_char(token, TOKEN_BRACE_OPEN);
        case '}':
            return token_char(token, TOKEN_BRACE_CLOSE);
        case '[':
            return token_char(token, TOKEN_BRACKET_OPEN);
        case ']':
            return token_char(token, TOKEN_BRACKET_CLOSE);
        case ':':
            return token_char(token, TOKEN_COLON);
        case ',':
            return token_char(token, TOKEN_COMMA);
        case '"':
            return token_string(token);
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return token_number(token);
        case 't':
            return token_special(token, TOKEN_TRUE, "true");
        case 'f':
            return token_special(token, TOKEN_FALSE, "false");
        case 'n':
            return token_special(token, TOKEN_NULL, "null");
        default:
            token->error = "Unknown Token";
            return false;
    }
}

static bool token_space(token *token)
{
    token->type = TOKEN_SPACE;
    while (true)
    {
        if (*token->end == ' ' || *token->end == '\n')
            token->end_location.column++;
        else if (*token->end == '\n')
        {
            token->end_location.line++;
            token->end_location.column = 0;
        }
        else
            break;
        token->end++;
    }
    token->end_location.offset += token->end - token->begin;
    return true;
}

static bool token_char(token *token, token_type type)
{
    token->type = type;
    token->end++;
    token->end_location.offset++;
    token->end_location.column++;
    return true;
}

static bool token_string(token *token)
{
    token->type = TOKEN_STRING;

    const char *data = token->begin + 1;
    bool escape = false;
    size_t column = token->location.column;

    while (true)
    {
        if (*data <= 0x1F || *data == 0x7F)
        {
            token->error = "Encountered Control character (possibly EOF) in string";
            return false;
        }

        if (*data == '\n')
        {
            token->end_location.line++;
            column = 0;
        }
        else
            column++;

        if (!escape)
        {
            if (*data == '\\')
                // We don't actually chech for valid escape sequences since the
                // actual parser needs to parse them anyway.
                escape = true;

            else if (*data == '"')
                break;
        }
        else
            escape = false;

        data++;
    }

    token->end = ++data;
    token->end_location.offset += data - token->begin;
    token->end_location.column = column;
    return true;
}

static bool token_number(token *token)
{
    token->type = TOKEN_INT;
    token->int_value = 0;

    const char *here = token->begin;

    bool negative = false;
    if (*here == '-')
    {
        negative = true;
        here++;
    }

    if (*here == '0')
        here++;
    else if (0x31 <= *here && *here <= 0x39) //[1-9]
    {
        long value = *here - 0x30;
        here++;

        while (true)
            if (0x30 <= *here && *here <= 0x39) //[0-9]
            {
                value *= 10;
                value += *here - 0x30;
                here++;
            }
            else
                break;

        token->int_value = value;
    }
    else
    {
        token->error = "Encountered broken number";
        return false;
    }

    if (negative)
        token->int_value *= -1;

    if (*here == '.')
    {
        token->type = TOKEN_FLOAT;
        token->float_value = token->int_value;
        here++;

        if (0x30 > *here || *here > 0x39) //[0-9]
        {
            token->error = "Encountered broken number (decimals)";
            return false;
        }

        const char *decimal_begin = here;
        long decimals = *here - 0x30;
        here++;

        while (true)
            if (0x30 <= *here && *here <= 0x39) //[0-9]
            {
                decimals *= 10;
                decimals += *here - 0x30;
                here++;
            }
            else
                break;

        long decimal_count = here - decimal_begin;
        double decimal_value = decimals * pow(10.0, -decimal_count);
        if (!negative)
            token->float_value += decimal_value;
        else
            token->float_value -= decimal_value;
    }

    if (*here == 'e' || *here == 'E')
    {
        if (token->type != TOKEN_FLOAT)
        {
            token->type = TOKEN_FLOAT;
            token->float_value = token->int_value;
        }
        here++;

        negative = false;
        switch (*here)
        {
        case '-':
            negative = true;
        case '+':
            here++;
        }

        if (0x30 > *here || *here > 0x39) //[0-9]
        {
            token->error = "Encountered broken number (exponent)";
            return false;
        }

        long exponent = *here - 0x30;
        here++;

        while (true)
            if (0x30 <= *here && *here <= 0x39) //[0-9]
            {
                exponent *= 10;
                exponent += *here - 0x30;
                here++;
            }
            else
                break;

        if (negative)
            exponent *= -1;

        token->float_value *= pow(10.0, exponent);
    }

    token->end = here;
    token->end_location.offset += here - token->begin;
    token->end_location.column += here - token->begin;
    return true;
}

static bool token_special(token *token, token_type type, const char *match)
{
    while (*match)
    {
        if (*(match++) != *(token->end++))
        {
            token->error = "Unknown Token";
            return false;
        }
        if (*match == 0x00)
            break;
    }
    token->end_location.offset += token->end - token->begin;
    token->end_location.column += token->end - token->begin;
    return true;
}
