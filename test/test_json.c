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

#include <libdy/dyj.h>
#include <libdy/json.h>
#include <libdy/dy.h>
#include <libdy/dy_error.h>

#include <stdio.h>
#include <inttypes.h>


//const char *json = "{1:2e2, \"hello\": [-1,3.0]}";


int tokentest(const char *json)
{
    dyj_token_t t;
    dyj_string_token_t st;
    dyj_init_token(&t, json);

    while (true)
    {
        if (!dyj_next_token(&t))
        {
            printf("ERROR: %s\n\tAt line %d, column %d\n", t.error,
                   t.error_location.line, t.error_location.column);
            return 1;
        }

        switch (t.type)
        {
            case TOKEN_EOF:
                return 0;
            case TOKEN_STRING:
                printf("STRING ");
                dyj_init_string(&st, &t);
                dyj_next_string(&st); // initial quote
                while (true)
                {
                    if (!dyj_next_string(&st))
                    {
                        printf("\nERROR: %s\n\tapprox. at column %d\n", st.error, t.location.column + st.error_offset);
                        return 1;
                    }

                    if (st.type == DYJ_STRTOK_QUOTE)
                        break;
                    else if (st.type == DYJ_STRTOK_TEXT)
                    {
                        const char *c = st.begin;
                        while (c < st.end)
                            putc(*(c++), stdout);
                    }
                    else if (st.type == DYJ_STRTOK_ESCAPE)
                    {
                        uint8_t utf[10];
                        size_t s = dyj_unicode_utf8(st.escape, utf);
                        for (size_t i = 0; i < s; ++i)
                            putc(utf[i], stdout);
                    }
                }
                putc(0x0a, stdout);
                break;
            case TOKEN_INT:
                printf("INT %"PRId64"\n", t.int_value);
                break;
            case TOKEN_FLOAT:
                printf("FLOAT %f\n", t.float_value);
                break;
            case TOKEN_BRACE_OPEN:
            case TOKEN_BRACE_CLOSE:
            case TOKEN_BRACKET_OPEN:
            case TOKEN_BRACKET_CLOSE:
            case TOKEN_COLON:
            case TOKEN_COMMA:
            case TOKEN_TRUE:
            case TOKEN_FALSE:
            case TOKEN_NULL:
                puts(dyj_token_names[t.type]);
            case TOKEN_SPACE:
                break;
            default:
                printf("Unknown token %d", t.type);
                return 1;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    if (tokentest(argv[1]))
        puts("===> tokentest failed.");

    puts("");


    DyObject *o = DyJson_Parse(argv[1]);

    DY_ERR_HANDLER
        DY_ERR_CATCH_ALL(e)
        {
            printf("[EE] %s: %s\n", DyErr_ErrId(e), DyErr_Message(e));
            DY_ERR_RETURN(1);
        }
    DY_ERR_HANDLER_END

    if(!o)
    {
        puts("NULL with no error set");
        return -1;
    }

    DyObject *s = Dy_Repr(o);
    puts(DyString_AsString(s));

    Dy_Release(s);
    Dy_Release(o);

    return 0;
}
