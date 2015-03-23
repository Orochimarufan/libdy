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

#include <libdy/dy.h>
#include <libdy/dy_error.h>
#include <libdy/dyj.h>
#include <libdy/json.h>

#include <stdio.h>


typedef struct input {
    const char *filename;
    char *buffer;
    size_t buffer_size;
    FILE* fd;
} input;


void next_chunk(dyj_token_t *token, input *in)
{
    fseek(in->fd, token->location.offset, SEEK_SET);
    size_t x = fread(in->buffer, 1, in->buffer_size, in->fd);
    if (x < in->buffer_size)
        in->buffer[x] = 0;
    dyj_next_chunk(token, in->buffer);
}


int main(int argc, char **argv)
{
    if (argc < 2)
        return 1;

    input in;
    in.filename = argv[1];
    in.buffer_size = 1024 * 512;
    in.buffer = malloc(in.buffer_size + 1);
    in.fd = fopen(in.filename, "r");

    size_t x = fread(in.buffer, 1, in.buffer_size, in.fd);
    if (x < in.buffer_size)
        in.buffer[x] = 0x00;
    else
        in.buffer[x] = 0x03;

    dyj_token_t tok;
    dyj_init_token(&tok, in.buffer);

    DyObject *o = DyJson_NextEx(&tok, (DyJson_NextChunkFn_t) next_chunk, &in);

    DY_ERR_HANDLER
        DY_ERR_CATCH_ALL(e)
        {
            printf("[EE] %s: %s", DyErr_ErrId(e), DyErr_Message(e));
            DY_ERR_ESCAPE return 2;
        }
    DY_ERR_HANDLER_END

    if (!o)
    {
        puts("NULL without exception");
        return 4;
    }

    DyObject *s = Dy_Repr(o);

    DY_ERR_HANDLER
        DY_ERR_CATCH_ALL(e)
        {
            printf("[EE] %s: %s",DyErr_ErrId(e), DyErr_Message(e));
            DY_ERR_ESCAPE return 3;
        }
    DY_ERR_HANDLER_END

    if (!s)
    {
        puts("NULL without exception");
        return 4;
    }

    puts(DyString_AsString(s));

    Dy_Release(s);
    Dy_Release(o);

    return 0;
}
