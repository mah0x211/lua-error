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
 */

#ifndef lua_error_h
#define lua_error_h

#include "lauxhlib.h"
#include <ctype.h>
#include <lua.h>
#include <string.h>
#include <unistd.h>

#define ERROR_MT      "error"
#define ERROR_TYPE_MT "error.type"

typedef struct {
    int ref_name;
} error_type_t;

typedef struct {
    // runtime error
    int ref;
    int ref_tostring;
    int ref_where;
    int ref_traceback;
    int ref_wrap;
    // error_type_t
    int ref_type;
} error_t;

LUALIB_API int luaopen_error(lua_State *L);
LUALIB_API int luaopen_error_type(lua_State *L);
LUALIB_API int luaopen_error_check(lua_State *L);

static inline error_type_t *new_error_type(lua_State *L, int nameidx)
{
    error_type_t *errt = NULL;
    size_t len         = 0;
    const char *name   = lauxh_checklstring(L, nameidx, &len);

    if (len == 0 || len > 127) {
        lauxh_argerror(L, nameidx,
                       "string length between 1-127 expected, got %zu", len);
        return NULL;
    } else if (!isalpha(*name)) {
        lauxh_argerror(
            L, nameidx,
            "first letter to be alphabetic character expected, got '%c'",
            *name);
        return NULL;
    }
    for (size_t i = 1; i < len; i++) {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '.') {
            lauxh_argerror(
                L, nameidx,
                "alphanumeric or '_' or '.' characters expected, got '%c'",
                name[i]);
            return NULL;
        }
    }

    // create new error_type_t
    errt           = lua_newuserdata(L, sizeof(error_type_t));
    errt->ref_name = lauxh_refat(L, nameidx);
    lauxh_setmetatable(L, ERROR_TYPE_MT);

    return errt;
}

static inline void get_whereis(lua_State *L, int level)
{
    lua_Debug ar;
    luaL_Buffer b;

    luaL_buffinit(L, &b);
    luaL_where(L, level);
    luaL_addvalue(&b);

    // get stackinfo
    if (lua_getstack(L, level, &ar)) {
        lua_getinfo(L, "Sln", &ar);
        // is there a name from code?
        if (*ar.namewhat) {
            lua_pushfstring(L, "in %s '%s'", ar.namewhat, ar.name);
        }
        // main
        else if (*ar.what == 'm') {
            lua_pushliteral(L, "in main chunk");
        }
        // not C function, use <file:line>
        else if (*ar.what == 'C') {
            lua_pushliteral(L, "?");
        } else {
            lua_pushfstring(L, "in function <%s:%d>", ar.short_src,
                            ar.linedefined);
        }
        luaL_addvalue(&b);

        if (*ar.what == 't') {
            luaL_addstring(&b, "(...tail calls...)");
        }
        luaL_addstring(&b, ": ");
    }

    luaL_pushresult(&b);
}

static inline error_t *new_error(lua_State *L, int idx, int level,
                                 int traceback)
{
    error_t *err = NULL;

    if (lua_isnoneornil(L, idx)) {
        goto INVALID_ARG;
    }
    err                = lua_newuserdata(L, sizeof(error_t));
    err->ref_tostring  = LUA_NOREF;
    err->ref_traceback = LUA_NOREF;
    err->ref_wrap      = LUA_NOREF;
    err->ref_type      = LUA_NOREF;

    // check argument
    switch (lua_type(L, idx)) {
    case LUA_TTABLE:
        lua_pushliteral(L, "tostring");
        lua_rawget(L, idx);
        if (lua_isfunction(L, -1)) {
            // get tostring function
            err->ref_tostring = lauxh_ref(L);
        } else {
            // get __tostring metamethod
            lua_pop(L, 1);
            if (lua_getmetatable(L, idx)) {
                lua_pushliteral(L, "__tostring");
                lua_rawget(L, -2);
            }
            if (!lua_isfunction(L, -1)) {
                luaL_argerror(L, idx,
                              "tostring function or __tostring metamethod "
                              "does not exist in table");
                return NULL;
            }
            err->ref_tostring = lauxh_ref(L);
            lua_pop(L, 1);
        }
    case LUA_TSTRING:
        break;

    default:
INVALID_ARG:
        lauxh_argerror(L, idx,
                       "argument must be string or table expected, got %s",
                       lua_typename(L, lua_type(L, idx)));
        return NULL;
    }

    err->ref = lauxh_refat(L, idx);
    get_whereis(L, level);
    err->ref_where = lauxh_ref(L);
    if (traceback) {
        lauxh_traceback(NULL, L, NULL, level);
        err->ref_traceback = lauxh_ref(L);
    }

    // create new error
    lauxh_setmetatable(L, ERROR_MT);

    return err;
}

#endif
