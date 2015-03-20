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

#include "dyj.h"

#include <math.h>
#include <string.h>


// -----------------------------------------------------------------------------
// JSON Tokenization
// -----------------------------------------------------------------------------
void dyj_init_token(dyj_token_t *token, const char *data)
{
    dyj_init_token_ex(token, data, 0, 1, 1);
}

void dyj_init_token_ex(dyj_token_t *token, const char *data, size_t offset, size_t line, size_t column)
{
    // Initialize token
    token->type = TOKEN_INVALID;
    token->begin = data;
    token->end = data;
    token->end_location.offset = offset;
    token->end_location.line = line;
    token->end_location.column = column;
    token->error = NULL;
}


static bool token_space(dyj_token_t *token);
static bool token_char(dyj_token_t *token, dyj_token_type type);
static bool token_string(dyj_token_t *token);
static bool token_number(dyj_token_t *token);
static bool token_special(dyj_token_t *token, dyj_token_type type, const char *match);


inline static bool token_error(dyj_token_t *token, const char *here, const char *message);
inline static bool token_end_of_chunk(dyj_token_t *token, const char *here);

inline static void update_token(dyj_token_t *token, dyj_token_type type, const char *here);


bool dyj_next_token(dyj_token_t *token)
{
    // Next token
    memcpy(&token->location, &token->end_location, sizeof(dyj_token_location));
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
            return token_error(token, token->begin, "Unknown Token");
    }
}

static bool token_space(dyj_token_t *token)
{
    token->type = TOKEN_SPACE;

    const char *here = token->begin;
    size_t line = token->location.line;
    size_t column = token->location.column;

    do if (*here == ' ' || *here == '\t')
            ++column;
        else if (*here == '\n')
        {
            ++line;
            column = 0;
        }
        else
            break;
    while (++here);

    token->end = here;
    token->end_location.offset += here - token->begin;
    token->end_location.line = line;
    token->end_location.column = column;

    return true;
}

static bool token_char(dyj_token_t *token, dyj_token_type type)
{
    token->type = type;
    token->end++;
    token->end_location.offset++;
    token->end_location.column++;

    return true;
}

static bool token_string(dyj_token_t *token)
{
    const char *data = token->begin + 1; // Skip the leading quote
    bool escape = false;

    do
    {
        if (*data <= 0x1F || *data == 0x7F)
            return token_error(token, data, "Encountered Control character (possibly EOF) in string");

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
    }
    while (++data);

    update_token(token, TOKEN_STRING, ++data); // Skip the trailing quote too

    return true;
}

static bool token_number(dyj_token_t *token)
{
    const char *here = token->begin;
    dyj_token_type type = TOKEN_INT;
    long value = 0;

    int negate = 1;
    if (*here == '-')
    {
        negate = -1;
        here++;
    }

    if (*here == '0')
        here++;
    else if (0x31 <= *here && *here <= 0x39) //[1-9]
    {
        value = *here - 0x30;
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
    }
    else
        return token_error(token, here, "Encountered broken number");

    double fvalue = value;

    if (*here == '.')
    {
        type = TOKEN_FLOAT;
        here++;

        if (0x30 > *here || *here > 0x39) //[0-9]
            return token_error(token, here, "Encountered broken number (decimals)");

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
        fvalue += decimals * pow(10.0, -decimal_count);
    }

    if (*here == 'e' || *here == 'E')
    {
        type = TOKEN_FLOAT;
        here++;

        int nexp = 1;
        switch (*here)
        {
        case '-':
            nexp = -1;
        case '+':
            here++;
        }

        if (0x30 > *here || *here > 0x39) //[0-9]
            return token_error(token, here, "Encountered broken number (exponent)");

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

        token->float_value *= pow(10.0, exponent * nexp);
    }

    // If we encounter ETX, we can't be sure the whole number has been read.
    if (token_end_of_chunk(token, here))
        return false;

    token->int_value = value * negate;
    token->float_value = fvalue * negate;

    update_token(token, type, here);

    return true;
}

static bool token_special(dyj_token_t *token, dyj_token_type type, const char *match)
{
    const char *here = token->begin;

    while (*match)
    {
        if (*(match++) != *(here++))
            return token_error(token, here, "Unknown Token");
    }

    update_token(token, type, here);

    return true;
}

inline static void update_location(dyj_token_location *loc, size_t size)
{
    loc->offset += size;
    loc->column += size;
}

inline static void token_set_error(dyj_token_t *token, const char *here, const char *message)
{
    token->type = TOKEN_INVALID;
    token->error = message;
    memcpy(&token->error_location, &token->location, sizeof(dyj_token_location));
    update_location(&token->error_location, here - token->begin);
}

inline static bool token_error(dyj_token_t *token, const char *here, const char *message)
{
    if (!token_end_of_chunk(token, here))
        token_set_error(token, here, message);
    return false;
}

inline static void update_token(dyj_token_t *token, dyj_token_type type, const char *here)
{
    token->type = type;
    token->end = here;

    update_location(&token->end_location, here - token->begin);
}


// -----------------------------------------------------------------------------
// Streaming
// -----------------------------------------------------------------------------
inline static bool token_end_of_chunk(dyj_token_t *token, const char *here)
{
    if (*here != 0x03) // ETX
        return false;

    token_set_error(token, here, dyj_chunk_end);
    return true;
}

const char *dyj_chunk_end = "End of Chunk (ETX)";

void dyj_next_chunk(dyj_token_t *token, const char *buffer)
{
    token->type = TOKEN_EOF;
    token->begin = token->end = buffer;
}


// -----------------------------------------------------------------------------
// JSON String Tokenization
// -----------------------------------------------------------------------------
void dyj_init_string(dyj_string_token_t *strtok, dyj_token_t *token)
{
    strtok->type = DYJ_STRTOK_INVALID;
    strtok->begin = strtok->end = token->begin;
    strtok->error = NULL;
}


inline static bool strtok_char(dyj_string_token_t *strtok, dyj_string_token_type type);
inline static bool strtok_escape(dyj_string_token_t *strtok);
inline static bool strtok_text(dyj_string_token_t *strtok);
inline static bool strtok_error(dyj_string_token_t *strtok, const char *here, const char *message);


bool dyj_next_string(dyj_string_token_t *strtok)
{
    strtok->begin = strtok->end;

    switch (*strtok->begin)
    {
    case '"':
        return strtok_char(strtok, DYJ_STRTOK_QUOTE);
    case '\\':
        return strtok_escape(strtok);
    default:
        return strtok_text(strtok);
    }
}


inline static bool strtok_char(dyj_string_token_t *strtok, dyj_string_token_type type)
{
    strtok->type = type;
    strtok->end++;
    return true;
}

inline static bool strtok_escape(dyj_string_token_t *strtok)
{
    const char *here = strtok->begin + 1;

    switch (*here)
    {
    case '"':
    case '/':
    case '\\':
        strtok->escape = *here;
        break;
    case 'b':
        strtok->escape = 0x08; // BS Backspace
        break;
    case 'f':
        strtok->escape = 0x0C; // FF Formfeed
        break;
    case 'n':
        strtok->escape = 0x0A; // LF Linefeed
        break;
    case 'r':
        strtok->escape = 0x0D; // CR Carriage return
        break;
    case 't':
        strtok->escape = 0x09; // HT Horizontal tab
        break;
    case 'u':
    {
        uint32_t escape = 0;

        for (int i = 0; i < 4; ++i)
        {
            here++;

            if (0x30 > *here || *here > 0x3F)
                return strtok_error(strtok, here, "Invalid unicode escape sequence");

            escape <<= 4;
            escape |= *here & 0xF;
        }

        strtok->escape = escape;
        break;
    }
    default:
        return strtok_error(strtok, here, "Unknown escape sequence");
    }

    strtok->type = DYJ_STRTOK_ESCAPE;
    strtok->end = ++here;
    return true;
}

inline static bool strtok_text(dyj_string_token_t *strtok)
{
    const char *here = strtok->begin;

    do here++;
    while (*here != '"' && *here != '\\');

    strtok->type = DYJ_STRTOK_TEXT;
    strtok->end = here;
    return true;
}

inline static bool strtok_error(dyj_string_token_t *strtok, const char *here, const char *message)
{
    strtok->type = DYJ_STRTOK_INVALID;
    strtok->error = message;
    strtok->error_offset = here - strtok->begin;
    return false;
}

int dyj_unicode_utf8(uint32_t codepoint, uint8_t *utf8)
{
    if (codepoint <= 0x7F)
    {
        utf8[0] = 0b01111111 & codepoint;
        return 1;
    }
    else if (codepoint <= 0x07FF)
    {
        utf8[1] = 0b10000000 | (0b00111111 & codepoint);
        utf8[0] = 0b11000000 | (0b00011111 & (codepoint >> 6));
        return 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        utf8[2] = 0b10000000 | (0b00111111 & codepoint);
        utf8[1] = 0b10000000 | (0b00111111 & (codepoint >> 6));
        utf8[0] = 0b11100000 | (0b00001111 & (codepoint >> 12));
        return 3;
    }
    else if (codepoint <= 0x001FFFFF)
    {
        utf8[3] = 0b10000000 | (0b00111111 & codepoint);
        utf8[2] = 0b10000000 | (0b00111111 & (codepoint >> 6));
        utf8[1] = 0b10000000 | (0b00111111 & (codepoint >> 12));
        utf8[0] = 0b11110000 | (0b00000111 & (codepoint >> 18));
        return 4;
    }
    else if (codepoint <= 0x03FFFFFF)
    {
        utf8[4] = 0b10000000 | (0b00111111 & codepoint);
        utf8[3] = 0b10000000 | (0b00111111 & (codepoint >> 6));
        utf8[2] = 0b10000000 | (0b00111111 & (codepoint >> 12));
        utf8[1] = 0b10000000 | (0b00111111 & (codepoint >> 18));
        utf8[0] = 0b11111000 | (0b00000011 & (codepoint >> 24));
        return 5;
    }
    else if (codepoint <= 0x7FFFFFFF)
    {
        utf8[5] = 0b10000000 | (0b00111111 & codepoint);
        utf8[4] = 0b10000000 | (0b00111111 & (codepoint >> 6));
        utf8[3] = 0b10000000 | (0b00111111 & (codepoint >> 12));
        utf8[2] = 0b10000000 | (0b00111111 & (codepoint >> 18));
        utf8[1] = 0b10000000 | (0b00111111 & (codepoint >> 24));
        utf8[0] = 0b11111100 | (0b00000001 & (codepoint >> 30));
        return 6;
    }
    else
        return -1;
}
