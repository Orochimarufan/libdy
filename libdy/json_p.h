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

#include "dy_defs.h"

#include <stdbool.h>
#include <stdlib.h>

// Types
typedef enum token_type {
    TOKEN_EOF,
    TOKEN_SPACE,
    TOKEN_BRACE_OPEN,
    TOKEN_BRACE_CLOSE,
    TOKEN_BRACKET_OPEN,
    TOKEN_BRACKET_CLOSE,
    TOKEN_COLON,
    TOKEN_COMMA,
    TOKEN_STRING,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_NULL,
    TOKEN_TRUE,
    TOKEN_FALSE,
} token_type;

typedef struct token_location {
    const char *filename;
    size_t offset;
    size_t line;
    size_t column;
} token_location;

typedef struct token {
    token_type type;
    const char *begin;
    const char *end;
    token_location location;
    token_location end_location;
    const char *error;
    // Not Implemented:
    long int_value;
    double float_value;
} token;

// Prototypes
bool first_token(token *token,
                 const char *data,
                 const char *filename);

bool first_token_ex(token *token,
                    const char *data,
                    const char *filename,
                    size_t offset,
                    size_t line,
                    size_t column);

bool next_token(token *token);

DyObject *parse_json(token *start);
