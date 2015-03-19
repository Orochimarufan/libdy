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

#include <libdy/json_p.h> // Private header!

#include <stdio.h>


//const char *json = "{1:2e2, \"hello\": [-1,3.0]}";


int main(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    token t;
    //init_token_ex(&t, json, "test_json.c", 0, 21, 20);
    init_token(&t, argv[1], "cmdline");

    while (true)
    {
        if (!next_token(&t))
        {
            printf("ERROR: %s\n\tapprox. at column %d\n", t.error, t.end_location.column);
            return 1;
        }

        switch (t.type)
        {
            case TOKEN_EOF:
                return 0;
            case TOKEN_BRACE_OPEN:
                puts("BRACE {");
                break;
            case TOKEN_BRACE_CLOSE:
                puts("BRACE }");
                break;
            case TOKEN_BRACKET_OPEN:
                puts("BRACK [");
                break;
            case TOKEN_BRACKET_CLOSE:
                puts("BRACK ]");
                break;
            case TOKEN_COLON:
                puts("COLON :");
                break;
            case TOKEN_COMMA:
                puts("COMMA ,");
                break;
            case TOKEN_STRING:
                printf("STRING ");
                for (const char *c = t.begin; c < t.end; ++c)
                    putc(*c, stdout);
                putc(0x0a, stdout);
                break;
            case TOKEN_INT:
                printf("INT %ld\n", t.int_value);
                break;
            case TOKEN_FLOAT:
                printf("FLOAT %f\n", t.float_value);
                break;
            case TOKEN_TRUE:
                puts("TRUE");
                break;
            case TOKEN_FALSE:
                puts("FALSE");
                break;
            case TOKEN_NULL:
                puts("NULL");
                break;
        }
    }
}
