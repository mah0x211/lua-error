/**
 *  Copyright (C) 2021 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 *
 */

#include "lua_error.h"

static int index_lua(lua_State *L)
{
    static const char *const fields[] = {
        "message", "type", "code", "op", NULL,
    };
    lua_error_t *err = luaL_checkudata(L, 1, LUA_ERROR_MT);
    int idx          = luaL_checkoption(L, 2, NULL, fields);

    switch (idx) {
    case 0:
        lauxh_pushref(L, err->ref_msg);
        return 1;

    case 1:
        lauxh_pushref(L, err->ref_type);
        return 1;

    case 2: {
        int code = -1;
        if (err->ref_type != LUA_NOREF) {
            lauxh_pushref(L, err->ref_type);
            code = ((lua_error_type_t *)lua_touserdata(L, -1))->code;
            lua_pop(L, 1);
        }
        lua_pushinteger(L, code);
        return 1;
    }

    case 3:
        if (err->ref_msg != LUA_NOREF) {
            lauxh_pushref(L, err->ref_msg);
            lauxh_pushref(
                L, ((lua_error_message_t *)lua_touserdata(L, -1))->ref_op);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int tostring_lua(lua_State *L)
{
    lua_error_t *err          = luaL_checkudata(L, 1, LUA_ERROR_MT);
    lua_error_message_t *errm = NULL;
    lua_error_type_t *errt    = NULL;
    int ref_typemsg           = LUA_NOREF;
    luaL_Buffer b             = {0};

    lauxh_pushref(L, err->ref_msg);
    errm = lua_touserdata(L, -1);
    if (err->ref_type != LUA_NOREF) {
        lauxh_pushref(L, err->ref_type);
        errt = lua_touserdata(L, -1);
    }

    lua_settop(L, 1);
    luaL_buffinit(L, &b);

    // <where>
    lauxh_pushref(L, err->ref_where);
    luaL_addvalue(&b);

    // [<type.name>:<type.code>]
    if (errt) {
        luaL_addchar(&b, '[');
        lauxh_pushref(L, errt->ref_name);
        luaL_addvalue(&b);
        luaL_addchar(&b, ':');
        lua_pushinteger(L, errt->code);
        lua_error_tostring(L, -1);
        luaL_addvalue(&b);
        luaL_addchar(&b, ']');
        ref_typemsg = errt->ref_msg;
    }

    // [<message.op>]
    if (errm->ref_op != LUA_NOREF) {
        luaL_addchar(&b, '[');
        lauxh_pushref(L, errm->ref_op);
        luaL_addvalue(&b);
        luaL_addchar(&b, ']');
    }

    if (errt || errm->ref_op != LUA_NOREF) {
        luaL_addchar(&b, ' ');
    }

    if (ref_typemsg == LUA_NOREF) {
        // <message.message>
        lauxh_pushref(L, errm->ref_msg);
        lua_error_tostring(L, -1);
        luaL_addvalue(&b);
    } else {
        // <type.message>
        lauxh_pushref(L, ref_typemsg);
        lua_error_tostring(L, -1);
        luaL_addvalue(&b);
        if (errm->ref_msg != LUA_REFNIL) {
            // use a message as sub-message as follows:
            //  <type.message> (<message>)
            luaL_addstring(&b, " (");
            lauxh_pushref(L, errm->ref_msg);
            lua_error_tostring(L, -1);
            luaL_addvalue(&b);
            luaL_addchar(&b, ')');
        }
    }

    if (err->ref_traceback != LUA_NOREF) {
        luaL_addchar(&b, '\n');
        lauxh_pushref(L, err->ref_traceback);
        luaL_addvalue(&b);
    }
    luaL_pushresult(&b);

    // convert wrapped errors to string
    if (err->ref_wrap != LUA_NOREF) {
        lua_pushliteral(L, "\n");
        lauxh_pushref(L, err->ref_wrap);
        if (!luaL_callmeta(L, -1, "__tostring")) {
            luaL_error(L, "error has no __tostring metamethod");
        } else if (!lua_isstring(L, -1)) {
            luaL_error(L, "error.__tostring metamethod does not "
                          "return a string");
        }
        lua_replace(L, -2);
    }

    // convert errors to string
    if (lua_gettop(L) > 2) {
        lua_concat(L, lua_gettop(L) - 1);
    }

    return 1;
}

static int gc_lua(lua_State *L)
{
    lua_error_t *err = lua_touserdata(L, 1);

    err->ref_msg       = lauxh_unref(L, err->ref_msg);
    err->ref_where     = lauxh_unref(L, err->ref_where);
    err->ref_traceback = lauxh_unref(L, err->ref_traceback);
    err->ref_wrap      = lauxh_unref(L, err->ref_wrap);
    err->ref_type      = lauxh_unref(L, err->ref_type);

    return 0;
}

typedef struct {
    int allocd;
    char *mem;
    union {
        lua_Integer i;
        lua_Number d;
        const char *s;
    } val;
} format_arg_t;

static int push_string(lua_State *L)
{
    char *str = (char *)lua_topointer(L, 1);
    lua_pushstring(L, str);
    return 1;
}

static int inline get_utf8len(unsigned char *s)
{
    if ((*s & 0x80) == 0) {
        // ASCII
        return 1;
    } else if ((*s & 0xE0) == 0xC0) {
        // 110xxxxx 10xxxxxx
        return 2;
    } else if ((*s & 0xF0) == 0xE0) {
        // 1110xxxx 10xxxxxx 10xxxxxx
        return 3;
    } else if ((*s & 0xF8) == 0xF0) {
        // 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
        return 4;
    }
    // invalid byte sequence
    return 1;
}

static void push_quoted_string(lua_State *L, int arg_idx)
{
    int top          = lua_gettop(L);
    size_t len       = 0;
    unsigned char *s = (unsigned char *)lauxh_tolstring(L, arg_idx, &len);
    luaL_Buffer b    = {};

    luaL_buffinit(L, &b);
    luaL_addchar(&b, '"');
    while (len--) {
        int utf8len = get_utf8len(s);
        if (utf8len > 1) {
            luaL_addlstring(&b, (char *)s, utf8len);
            s += utf8len;
            len -= utf8len - 1;
            continue;
        }

        if (*s == '"' || *s == '\\') {
            luaL_addchar(&b, '\\');
            luaL_addchar(&b, *s);
        } else if (!iscntrl(*s)) {
            luaL_addchar(&b, *s);
        } else {
            switch (*s) {
            case 0:
                luaL_addstring(&b, "\\0");
                break;
            case 7:
                luaL_addstring(&b, "\\a");
                break;
            case 8:
                luaL_addstring(&b, "\\b");
                break;
            case 9:
                luaL_addstring(&b, "\\t");
                break;
            case 10:
                luaL_addstring(&b, "\\n");
                break;
            case 11:
                luaL_addstring(&b, "\\v");
                break;
            case 12:
                luaL_addstring(&b, "\\f");
                break;
            case 13:
                luaL_addstring(&b, "\\r");
                break;

            default: {
                char buf[10];
                if (!isdigit(*(s + 1))) {
                    snprintf(buf, sizeof(buf), "\\%d", (int)*s);
                } else {
                    snprintf(buf, sizeof(buf), "\\%03d", (int)*s);
                }
                luaL_addstring(&b, buf);
            } break;
            }
        }
        s++;
    }
    luaL_addchar(&b, '"');
    luaL_pushresult(&b);
    lua_replace(L, top + 1);
    lua_settop(L, top + 1);
    return;
}

static void push_format_string(lua_State *L, const char *fmt, int type,
                               int arg_idx)
{
    union {
        lua_Integer i;
        lua_Number d;
        const char *s;
        const void *p;
    } val;
    char *mem = NULL;
    int rc    = 0;

    switch (type) {
    case 'd': // int (decimal)
    case 'i': // int (decimal) (same as 'd')
    case 'o': // unsigned int (octal)
    case 'u': // unsigned int (decimal)
    case 'x': // unsigned int (hexadecimal)
    case 'X': // unsigned int (hexadecimal) (uppercase)
        if (lua_type(L, arg_idx) == LUA_TBOOLEAN) {
            val.i = lua_toboolean(L, arg_idx);
        } else {
            val.i = luaL_checkinteger(L, arg_idx);
        }
        if (asprintf(&mem, fmt, val.i) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'c': // int (character)
        if (lua_type(L, arg_idx) == LUA_TSTRING) {
            size_t slen   = 0;
            const char *s = lua_tolstring(L, arg_idx, &slen);
            if (slen > 1) {
                luaL_argerror(L, arg_idx, "string length <=1 expected");
            }
            val.i = *s;
        } else {
            val.i = luaL_checkinteger(L, arg_idx);
        }
        if (asprintf(&mem, fmt, val.i) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'e': // double (scientific)
    case 'E': // double (scientific) (uppercase)
    case 'f': // double (decimal)
    case 'F': // double (decimal) (uppercase)
    case 'g': // double (scientific or decimal)
    case 'G': // double (scientific or decimal) (uppercase)
    case 'a': // double (hexadecimal) (C99)
    case 'A': // double (hexadecimal) (C99) (uppercase)
        val.d = luaL_checknumber(L, arg_idx);
        if (asprintf(&mem, fmt, val.d) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 's': { // any (string)
        int top = lua_gettop(L);
        val.s   = lauxh_tostring(L, arg_idx);
        if (asprintf(&mem, fmt, val.s) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        lua_settop(L, top);
    } break;

    case 'p': // void * (pointer)
        val.p = lua_topointer(L, arg_idx);
        if (asprintf(&mem, fmt, val.p) == -1) {
            luaL_error(L, "failed to asprintf: %s", strerror(errno));
        }
        break;

    case 'q': // any (quoted string)
        if (fmt[0] != '%' || fmt[1] != 'q' || fmt[2]) {
            luaL_error(L, "specifier '%%q' cannot have modifiers");
        }
        push_quoted_string(L, arg_idx);
        return;
    }

    lua_pushcfunction(L, push_string);
    lua_pushlightuserdata(L, mem);
    rc = lua_pcall(L, 1, 1, 0);
    // free allocated string
    free(mem);

    if (rc != 0) {
        // rethrow error with error message in stack top
        lua_error(L);
    }
}

static inline int uint2str(lua_State *L, char *buf, size_t len,
                           const char *placeholder, const int narg, int idx)
{
    if (idx > narg) {
        luaL_error(L,
                   "not enough arguments for placeholder '%s' in format string",
                   placeholder);
    }

    luaL_checktype(L, idx, LUA_TNUMBER);
    // convert argument to string as integer
    return snprintf(buf, len, "%d", (int)lua_tonumber(L, idx));
}

static int format_lua(lua_State *L)
{
    int narg         = lua_gettop(L);
    const char *fmt  = luaL_checkstring(L, 1);
    const char *head = fmt;
    const char *cur  = head;
    int nextarg      = 1;
    int top          = 0;

    // parse format specifiers
    while (*cur) {
        if (*cur == '%') {
            char buf[255]     = {0};
            size_t blen       = sizeof(buf);
            char *placeholder = buf;

#define COPY2PLACEHOLDER(str, len)                                             \
    do {                                                                       \
        size_t slen = (len);                                                   \
        if (slen >= blen) {                                                    \
            return luaL_error(L,                                               \
                              "each placeholder must be less than %d "         \
                              "characters in format string '%s'",              \
                              sizeof(placeholder), placeholder);               \
        }                                                                      \
        blen -= slen;                                                          \
        memcpy(placeholder, (str), slen);                                      \
        placeholder += slen;                                                   \
    } while (0)

            if (cur[1] == '%') {
                lua_pushlstring(L, head, cur - head + 1);
                // skip '%%' escape sequence
                cur += 2;
                head = cur;
                continue;
            }

            // push leading format string
            if (cur != head) {
                lua_pushlstring(L, head, cur - head);
            }
            fmt  = cur;
            head = cur;
            cur++;

            // flags field
            while (strchr("#I0- +'", *cur)) {
                cur++;
            }

            // int n_bits = sizeof(int) * 8;
            // int max_digits = n_bits / 3;
            // int buffer_size = max_digits + 2 + 1;
#define DYNSIZE (sizeof(int) * CHAR_BIT / 3 + 3)

            // width field
            while (strchr("1234567890*", *cur)) {
                if (*cur == '*') {
                    int wlen              = DYNSIZE;
                    const char w[DYNSIZE] = {0};

                    // copy leading format string
                    COPY2PLACEHOLDER(head, cur - head);
                    // skip '*'
                    head = cur + 1;

                    // get width from argument
                    nextarg++;
                    wlen = uint2str(L, (char *)w, (size_t)wlen, fmt, narg,
                                    nextarg);
                    // copy it to placeholder
                    COPY2PLACEHOLDER(w, wlen);
                }
                cur++;
            }

            // precision field
            if (*cur == '.') {
                // skip '.'
                cur++;
                while (strchr("1234567890*", *cur)) {
                    if (*cur == '*') {
                        int wlen              = DYNSIZE;
                        const char w[DYNSIZE] = {0};

                        // copy leading format string
                        COPY2PLACEHOLDER(head, cur - head);
                        // skip '*'
                        head = cur + 1;

                        // get precision from argument
                        nextarg++;
                        wlen = uint2str(L, (char *)w, wlen, fmt, narg, nextarg);
                        // copy it to placeholder
                        COPY2PLACEHOLDER(w, wlen);
                    }
                    cur++;
                }
            }

#undef DYNSIZE

            // length modifier
            if (strchr("hljztL", *cur)) {
                cur++;
            }

            // type field
            if (!strchr("diouxXeEfFgGaAcspqm", *cur)) {
                return luaL_error(L,
                                  "unsupported type field at '%c' in "
                                  "format string '%s'",
                                  *cur, fmt);
            }

            // copy leading format string
            COPY2PLACEHOLDER(head, cur - head + 1);
            head = cur + 1;

            if (*cur == 'm') {
                // printf %m is printed as strerror(errno) without params
                lua_checkstack(L, 1);
                lua_pushstring(L, strerror(errno));
            } else {
                nextarg++;
                if (nextarg > narg) {
                    return luaL_error(L,
                                      "not enough arguments for placeholder "
                                      "'%s' in format string",
                                      buf);
                }
                push_format_string(L, buf, *cur, nextarg);
            }
        }
        cur++;
    }

#undef COPY2PLACEHOLDER

    // push trailing format string
    if (cur > head) {
        lua_pushlstring(L, head, cur - head);
    }

    // if err argument is passed but not an error object, convert it to string
    // and concat to error message
    if (nextarg < narg && !lua_isnoneornil(L, nextarg + 1) &&
        !lauxh_isuserdataof(L, nextarg + 1, LUA_ERROR_MT)) {
        push_format_string(L, ": %s", 's', nextarg + 1);
        lua_pushnil(L);
        lua_replace(L, nextarg + 1);
    }

    // concat all strings
    lua_concat(L, lua_gettop(L) - narg);
    // replace format string with concatenated string
    lua_replace(L, 1);
    // remove all format arguments
    for (top = 1; nextarg < narg; top++) {
        nextarg++;
        lua_insert(L, 2);
    }
    lua_settop(L, top);

    return lua_error_new(L, 1);
}

static int new_lua(lua_State *L)
{
    return lua_error_new(L, 1);
}

static int toerror_lua(lua_State *L)
{
    // return passed error object
    if (lauxh_isuserdataof(L, 1, LUA_ERROR_MT)) {
        lua_settop(L, 1);
        return 1;
    }

    // create new error
    return new_lua(L);
}

static int unwrap_lua(lua_State *L)
{
    lua_error_t *err = luaL_checkudata(L, 1, LUA_ERROR_MT);

    lua_settop(L, 1);
    lauxh_pushref(L, err->ref_wrap);

    return 1;
}

static int cause_lua(lua_State *L)
{
    int is_msg = 0;

    lua_settop(L, 1);
    if (lua_isnoneornil(L, 1)) {
        lua_pushnil(L);
    } else if (lauxh_isuserdataof(L, 1, LUA_ERROR_MT)) {
        lauxh_pushref(L, ((lua_error_t *)lua_touserdata(L, 1))->ref_msg);
        is_msg = 1;
    } else {
        is_msg = lauxh_isuserdataof(L, 1, LUA_ERROR_MESSAGE_MT);
    }

    lua_pushboolean(L, is_msg);
    return 2;
}

static int is_lua(lua_State *L)
{
    lua_error_t *err          = NULL;
    lua_error_message_t *errm = NULL;
    size_t len                = 0;
    const char *target        = NULL;
    void *label               = &&CMP_MESSAGE;

    // do nothing if target is nil or first argument is not an error object
    if (lua_isnoneornil(L, 2) || !lauxh_isuserdataof(L, 1, LUA_ERROR_MT)) {
        lua_pushnil(L);
        return 1;
    }
    lua_settop(L, 2);
    lua_insert(L, 1);
    err = (lua_error_t *)lua_touserdata(L, -1);

    // check target argument
    switch (lua_type(L, 1)) {
    case LUA_TSTRING:
        target = lua_tolstring(L, 1, &len);
        label  = &&CMP_STRING;
        break;

    case LUA_TUSERDATA:
        if (lauxh_isuserdataof(L, 1, LUA_ERROR_MT)) {
            label = &&CMP_ERROR;
        } else if (lauxh_isuserdataof(L, 1, LUA_ERROR_TYPE_MT)) {
            // compare with error.type
            label = &&CMP_ERROR_TYPE;
        } else if (lauxh_isuserdataof(L, 1, LUA_ERROR_MESSAGE_MT)) {
            // compare with error.message
            label = &&CMP_ERROR_MESSAGE;
        }
        break;
    }
    goto *label;

CMP_ERROR:
    if (lua_rawequal(L, 1, -1)) {
        return 1;
    }
    goto UNWRAP;

CMP_ERROR_TYPE:
    lauxh_pushref(L, err->ref_type);
    if (lua_rawequal(L, 1, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_pop(L, 1);
    goto UNWRAP;

CMP_ERROR_MESSAGE:
    lauxh_pushref(L, err->ref_msg);
    if (lua_rawequal(L, 1, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_pop(L, 1);
    goto UNWRAP;

CMP_MESSAGE:
    lauxh_pushref(L, err->ref_msg);
    errm = lua_touserdata(L, -1);
    lauxh_pushref(L, errm->ref_msg);
    if (lua_rawequal(L, 1, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    lua_pop(L, 2);
    goto UNWRAP;

CMP_STRING:
    lauxh_pushref(L, err->ref_msg);
    errm = lua_touserdata(L, -1);
    lauxh_pushref(L, errm->ref_msg);
    if (lua_type(L, -1) == LUA_TSTRING) {
        size_t mlen     = 0;
        const char *msg = lua_tolstring(L, -1, &mlen);
        if (mlen == len && strcmp(target, msg) == 0) {
            lua_pop(L, 2);
            return 1;
        }
    }
    lua_pop(L, 2);

UNWRAP:
    // check wrapped error
    if (err->ref_wrap != LUA_NOREF) {
        lauxh_pushref(L, err->ref_wrap);
        err = lua_touserdata(L, -1);
        lua_replace(L, -2);
        goto *label;
    }

    // not found
    lua_pushnil(L);
    return 1;
}

static int typeof_lua(lua_State *L)
{
    if (lauxh_ismetatableof(L, 1, LUA_ERROR_MT)) {
        lua_error_t *err = lua_touserdata(L, 1);
        lauxh_pushref(L, err->ref_type);
    } else if (lauxh_ismetatableof(L, 1, LUA_ERROR_TYPE_MT)) {
        lua_settop(L, 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int debug_lua(lua_State *L)
{
    lua_pushboolean(L, lauxh_checkboolean(L, 1));
    lua_setfield(L, LUA_REGISTRYINDEX, LUA_ERROR_DEBUG_FLG);
    return 0;
}

static int call_lua(lua_State *L)
{
    int level = (int)luaL_optinteger(L, 3, 1);

    lua_settop(L, 2);
    if (lua_type(L, 2) == LUA_TSTRING && level > 0) {
        luaL_where(L, level);
        lua_pushvalue(L, 2);
        lua_concat(L, 2);
    }

    return lua_error(L);
}

LUALIB_API int le_open_error_type(lua_State *L);
LUALIB_API int le_open_error_check(lua_State *L);
LUALIB_API int le_open_error_message(lua_State *L);

LUALIB_API int luaopen_error(lua_State *L)
{
    struct luaL_Reg mmethod[] = {
        {"__gc",       gc_lua      },
        {"__tostring", tostring_lua},
        {"__index",    index_lua   },
        {NULL,         NULL        }
    };
    struct luaL_Reg funcs[] = {
        {"debug",   debug_lua  },
        {"typeof",  typeof_lua },
        {"is",      is_lua     },
        {"cause",   cause_lua  },
        {"unwrap",  unwrap_lua },
        {"toerror", toerror_lua},
        {"new",     new_lua    },
        {"format",  format_lua },
        {NULL,      NULL       }
    };

    // create metatable
    luaL_newmetatable(L, LUA_ERROR_MT);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // metamethods
    for (struct luaL_Reg *ptr = mmethod; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    lua_pop(L, 1);

    // set default debug flag to false
    lua_pushboolean(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, LUA_ERROR_DEBUG_FLG);

    // export funcs
    lua_newtable(L);
    for (struct luaL_Reg *ptr = funcs; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    // export submodules
    lua_pushliteral(L, "type");
    le_open_error_type(L);
    lua_rawset(L, -3);
    lua_pushliteral(L, "message");
    le_open_error_message(L);
    lua_rawset(L, -3);
    lua_pushliteral(L, "check");
    le_open_error_check(L);
    lua_rawset(L, -3);

    // set metatable
    lua_newtable(L);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // export a call metamethod that equivalent to built-in error function
    lauxh_pushfn2tbl(L, "__call", call_lua);
    lua_setmetatable(L, -2);

    return 1;
}
