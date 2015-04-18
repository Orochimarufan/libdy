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

/**
 * @file json.h
 * @brief libdy JSON parser
 */

#pragma once

#include "types.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif


#define DY_ERRID_JSON_PARSE "dy.json.ParseError"
#define DY_ERRID_JSON_TOKEN "dy.json.ParseError.TokenError"
#define DY_ERRID_JSON_PARSE_STRING "dy.json.ParseError.StringParseError"


struct dyj_token_t;

typedef void(*DyJson_NextChunkFn_t)(struct dyj_token_t *token, void *data);

/**
 * @brief Parse json from a Token stream
 * @param token The first token in the stream
 * @param next_chunk A function to get the next input chunk
 * @param next_chunk_data Data pointer passed to next_chunk()
 * @return A new DyObject reference
 * @sa DyJson_NextEx
 * @sa DyJson_Parse
 */
LIBDY_API DyObject *DyJson_ThisEx(struct dyj_token_t *token,
						          DyJson_NextChunkFn_t next_chunk,
								  void *next_chunk_data);

/**
 * @brief Parse json from a Token stream
 * @param token A stream pointing _before_ the first token
 * @param next_chunk A function to get the next input chunk
 * @param next_chunk_data Data pointer passed to next_chunk()
 * @return A new DyObject reference
 * @sa dyj_next_chunk
 * @sa DyJson_ThisEx
 * @sa DyJson_Parse
 */
LIBDY_API DyObject *DyJson_NextEx(struct dyj_token_t *token,
                                  DyJson_NextChunkFn_t next_chunk,
                                  void *next_chunk_data);

/**
 * @brief Parse a json buffer into a DyObject
 * @param json The json data buffer
 * @return A new DyObject reference
 * @sa DyJson_ParseEx
 */
LIBDY_API DyObject *DyJson_Parse(const char *json);


#ifdef __cplusplus
}
#endif
