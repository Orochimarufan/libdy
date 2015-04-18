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

#include "config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// -----------------------------------------------------------------------------
// JSON Tokenization
// -----------------------------------------------------------------------------
typedef enum dyj_token_type {
    TOKEN_INVALID,
    TOKEN_EOF,
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
    TOKEN_SPACE,
} dyj_token_type;

extern LIBDY_API const char *dyj_token_names[];

typedef struct dyj_token_location {
    size_t offset;
    size_t line;
    size_t column;
} dyj_token_location;

/**
 * @class dyj_token_t
 * @brief A DyJSON token
 */
typedef struct dyj_token_t {
    // Token type
    dyj_token_type type;

    // Token buffer
    const char *begin;
    const char *end;

    // Number Values
    int64_t int_value;
    double float_value;

    // Location tracking
    dyj_token_location location;
    dyj_token_location end_location;

    // Error message
    const char *error;
    dyj_token_location error_location;
} dyj_token_t;


/**
 * @brief Initialize the token structure
 * @param token The token
 * @param buffer A pointer to the start of the json data
 */
LIBDY_API void dyj_init_token(dyj_token_t *token,
                              const char *buffer);

/**
 * @brief Initialize the token structure
 * @param token The token
 * @param buffer A pointer to the start of the json data
 * @param offset The initial offset into the file
 * @param line The initial line number
 * @param column The initial column
 */
LIBDY_API void dyj_init_token_ex(dyj_token_t *token,
                                 const char *buffer,
                                 size_t offset,
                                 size_t line,
                                 size_t column);

/**
 * @brief Get the next token
 * @param token The token structure
 * @return false if it failed, setting a message in token->error
 */
LIBDY_API bool dyj_next_token(dyj_token_t *token);


// -----------------------------------------------------------------------------
// Streaming
// -----------------------------------------------------------------------------
/**
 * @brief special value for token->error
 * Indicates that the chunk sentinel ETX was reached
 */
extern LIBDY_API const char *dyj_chunk_end;

/**
 * @brief Continue parsing in a new chunk.
 * @param token The token
 * @param buffer The new buffer
 * A chunk must always end in a ETX byte.
 * The new chunk must contain everything starting with the next token.
 * This means that the new chunk starts with the same data as token->end.
 * (chunk boundaries may only occur between tokens)
 * You can copy the remaining data from token->end or read it from your source
 *  starting at token->end_location.offset
 */
LIBDY_API void dyj_next_chunk(dyj_token_t *token,
                              const char *buffer);


// -----------------------------------------------------------------------------
// JSON String Tokenization
// -----------------------------------------------------------------------------
typedef enum dyj_string_token_type {
    DYJ_STRTOK_INVALID,
    DYJ_STRTOK_QUOTE,
    DYJ_STRTOK_TEXT,
    DYJ_STRTOK_ESCAPE,
} dyj_string_token_type;

typedef struct dyj_string_token_t {
    // Type
    dyj_string_token_type type;

    // Buffer
    const char *begin;
    const char *end;

    // Unicode codepoint for escape sequence
    uint32_t escape;

    // Error message
    const char *error;
    size_t error_offset;
} dyj_string_token_t;

/**
 * @brief Initialize a JSON string token
 * @param strtok The string token
 * @param token A string-type JSON token
 */
LIBDY_API void dyj_init_string(dyj_string_token_t *strtok,
                               dyj_token_t *token);

/**
 * @brief Get the next JSON string token
 * @param strtok The string token
 * @return Whether the operation succeeded
 */
LIBDY_API bool dyj_next_string(dyj_string_token_t *strtok);

/**
 * @brief Convert an Unicode codepoint to utf8 bytes
 * @param codepoint The codepoint
 * @param utf8 UTF-8 output buffer, should have at least 7 bytes of space
 * @return the number of bytes written
 * Writes the utf8 code units for \c codepoint to \c utf8
 */
LIBDY_API int dyj_unicode_utf8(uint32_t codepoint, uint8_t *utf8);
