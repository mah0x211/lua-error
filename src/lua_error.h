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

#define LE_ERROR_MT      "error"
#define LE_ERROR_TYPE_MT "error.type"

typedef struct {
    int ref_name;
} le_error_type_t;

typedef struct {
    // runtime error
    int ref_msg;
    int ref_tostring;
    int ref_where;
    int ref_traceback;
    int ref_wrap;
    // error_type_t
    int ref_type;
} le_error_t;

static inline void le_loadlib(lua_State *L)
{
    int top = lua_gettop(L);
    luaL_getmetatable(L, LE_ERROR_MT);
    if (lua_isnil(L, -1) &&
        ((luaL_loadstring(L, "require('error')") || lua_pcall(L, 0, 0, 0)))) {
        lua_error(L);
    }
    lua_settop(L, top);
}

static inline void le_where(lua_State *L, int level)
{
    lua_Debug ar;
    luaL_Buffer b;

    luaL_buffinit(L, &b);
    luaL_where(L, level);
    luaL_addvalue(&b);

    // get stackinfo
    if (lua_getstack(L, level, &ar)) {
        lua_getinfo(L, "Sln", &ar);
        if (*ar.namewhat) {
            // got the name from code
            lua_pushfstring(L, "in %s '%s'", ar.namewhat, ar.name);
        } else if (*ar.what == 'm') {
            // main
            lua_pushliteral(L, "in main chunk");
        } else if (*ar.what == 'C') {
            // C function
            lua_pushliteral(L, "?");
        } else {
            // lua function, use <file:line>
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

// create a new error that equivalent to the following code;
//
//  error.new(msg [, wrap [, level [, traceback]]])
//
static inline int le_new_error(lua_State *L, int msgidx)
{
    le_loadlib(L);
    int idx          = (msgidx < 0) ? lua_gettop(L) + msgidx + 1 : msgidx;
    le_error_t *err  = NULL;
    le_error_t *wrap = lauxh_optudata(L, idx + 1, LE_ERROR_MT, NULL);
    int level        = (int)lauxh_optuint8(L, idx + 2, 1);
    int traceback    = lauxh_optboolean(L, idx + 3, 0);

    if (wrap) {
        lua_settop(L, idx + 1);
    }
    if (lua_isnoneornil(L, idx)) {
        goto INVALID_ARG;
    }
    err                = lua_newuserdata(L, sizeof(le_error_t));
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
                return luaL_argerror(
                    L, idx,
                    "tostring function or __tostring metamethod "
                    "does not exist in table");
            }
            err->ref_tostring = lauxh_ref(L);
            lua_pop(L, 1);
        }
    case LUA_TSTRING:
        break;

    default:
INVALID_ARG:
        return lauxh_argerror(
            L, idx, "argument must be string or table expected, got %s",
            lua_typename(L, lua_type(L, idx)));
    }

    err->ref_msg = lauxh_refat(L, idx);
    // with wrap argument
    if (wrap) {
        err->ref_wrap = lauxh_refat(L, idx + 1);
    }
    le_where(L, level);
    err->ref_where = lauxh_ref(L);
    if (traceback) {
        lauxh_traceback(NULL, L, NULL, level);
        err->ref_traceback = lauxh_ref(L);
    }
    // create new error
    lauxh_setmetatable(L, LE_ERROR_MT);

    return 1;
}

// create a new error type that equivalent to the following code;
//
//  error.type.new(name)
//
static inline int le_new_error_type(lua_State *L, int nameidx)
{
    le_loadlib(L);
    int idx = (nameidx < 0) ? lua_gettop(L) + nameidx + 1 : nameidx;
    le_error_type_t *errt = NULL;
    size_t len            = 0;
    const char *name      = lauxh_checklstring(L, idx, &len);

    if (len == 0 || len > 127) {
        return lauxh_argerror(
            L, idx, "string length between 1-127 expected, got %zu", len);
    } else if (!isalpha(*name)) {
        return lauxh_argerror(
            L, idx,
            "first letter to be alphabetic character expected, got '%c'",
            *name);
    }
    for (size_t i = 1; i < len; i++) {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '.') {
            return lauxh_argerror(
                L, idx,
                "alphanumeric or '_' or '.' characters expected, got '%c'",
                name[i]);
        }
    }

    // create new le_error_type_t
    errt           = lua_newuserdata(L, sizeof(le_error_type_t));
    errt->ref_name = lauxh_refat(L, idx);
    lauxh_setmetatable(L, LE_ERROR_TYPE_MT);

    return 1;
}

// create a new typed error that equivalent to the following code;
//
//  <myerr>:new(msg [, wrap [, level [, traceback]]])
//
static inline int le_new_type_error(lua_State *L, int typeidx)
{
    le_loadlib(L);
    int idx = (typeidx < 0) ? lua_gettop(L) + typeidx + 1 : typeidx;

    luaL_checkudata(L, idx, LE_ERROR_TYPE_MT);
    le_new_error(L, idx + 1);
    ((le_error_t *)lua_touserdata(L, -1))->ref_type = lauxh_refat(L, idx);

    return 1;
}

#endif
