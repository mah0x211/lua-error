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

static void tostring(lua_State *L, le_error_t *err)
{
    if (err->ref_tostring != LUA_NOREF) {
        lauxh_pushref(L, err->ref_tostring);
        lauxh_pushref(L, err->ref_msg);
        lauxh_pushref(L, err->ref_where);
        lauxh_pushref(L, err->ref_traceback);
        lauxh_pushref(L, err->ref_type);
        lua_call(L, 4, 1);
    } else {
        int top = lua_gettop(L);

        lauxh_pushref(L, err->ref_where);
        if (err->ref_type == LUA_NOREF) {
            lauxh_pushref(L, err->ref_msg);
        } else {
            const char *name      = NULL;
            le_error_type_t *errt = NULL;

            lauxh_pushref(L, err->ref_type);
            errt = lua_touserdata(L, -1);
            lua_pop(L, 1);

            lauxh_pushref(L, errt->ref_name);
            name = lua_tostring(L, -1);
            lua_pop(L, 1);
            lua_pushfstring(L, "[%s] ", name);
            if (errt->ref_msg == LUA_NOREF) {
                lauxh_pushref(L, err->ref_msg);
            } else {
                // use a type message as main message
                lauxh_pushref(L, errt->ref_msg);
                if (err->ref_msg != LUA_NOREF) {
                    lua_pushliteral(L, " (");
                    lauxh_pushref(L, err->ref_msg);
                    lua_pushliteral(L, ")");
                }
            }
        }

        if (err->ref_traceback != LUA_NOREF) {
            lua_pushliteral(L, "\n");
            lauxh_pushref(L, err->ref_traceback);
        }
        lua_concat(L, lua_gettop(L) - top);
    }

    // convert wrapped errors to string
    if (err->ref_wrap != LUA_NOREF) {
        lua_pushliteral(L, "\n");
        le_error_t *werr = NULL;
        lauxh_pushref(L, err->ref_wrap);
        werr = lua_touserdata(L, -1);
        lua_pop(L, 1);
        tostring(L, werr);
    }
}

static int tostring_lua(lua_State *L)
{
    le_error_t *err = luaL_checkudata(L, 1, LE_ERROR_MT);
    int nstr        = 0;

    lua_settop(L, 1);
    // convert errors to string
    tostring(L, err);
    nstr = lua_gettop(L) - 1;
    if (nstr > 1) {
        lua_concat(L, nstr);
    }

    return 1;
}

static int gc_lua(lua_State *L)
{
    le_error_t *err = lua_touserdata(L, 1);

    err->ref_msg       = lauxh_unref(L, err->ref_msg);
    err->ref_tostring  = lauxh_unref(L, err->ref_tostring);
    err->ref_where     = lauxh_unref(L, err->ref_where);
    err->ref_traceback = lauxh_unref(L, err->ref_traceback);
    err->ref_wrap      = lauxh_unref(L, err->ref_wrap);
    err->ref_type      = lauxh_unref(L, err->ref_type);

    return 0;
}

static int new_lua(lua_State *L)
{
    return le_new_error(L, 1);
}

static int toerror_lua(lua_State *L)
{
    // return passed error object
    if (lauxh_isuserdataof(L, 1, LE_ERROR_MT)) {
        lua_settop(L, 1);
        return 1;
    }

    // create new error
    return new_lua(L);
}

static int unwrap_lua(lua_State *L)
{
    le_error_t *err = luaL_checkudata(L, 1, LE_ERROR_MT);

    lua_settop(L, 1);
    lauxh_pushref(L, err->ref_wrap);

    return 1;
}

static int cause_lua(lua_State *L)
{
    if (lua_gettop(L) == 0) {
        lua_pushnil(L);
    } else if (lauxh_ismetatableof(L, 1, LE_ERROR_MT)) {
        lauxh_pushref(L, ((le_error_t *)lua_touserdata(L, 1))->ref_msg);
        return 1;
    }
    lua_settop(L, 1);
    return 1;
}

static int is_lua(lua_State *L)
{
    int top          = lua_gettop(L);
    le_error_t *err  = NULL;
    uintptr_t src    = 0;
    uintptr_t target = 0;
    void *label      = NULL;

    // do nothing if first argument is not an error object
    if (top < 2 || !lauxh_isuserdataof(L, 1, LE_ERROR_MT)) {
        lua_pushnil(L);
        return 1;
    }
    err    = (le_error_t *)lua_touserdata(L, 1);
    src    = (uintptr_t)err;
    target = (uintptr_t)lua_topointer(L, 2);

    // check target argument
    switch (lua_type(L, 2)) {
    case LUA_TTABLE:
        label = &&COMPARE_TABLE;
        break;

    case LUA_TSTRING:
        target = (uintptr_t)lua_tostring(L, 2);
        label  = &&COMPARE_STRING;
        break;

    case LUA_TUSERDATA:
        if (lauxh_isuserdataof(L, 2, LE_ERROR_MT)) {
            label = &&COMPARE;
            break;
        } else if (lauxh_isuserdataof(L, 2, LE_ERROR_TYPE_MT)) {
            label = &&COMPARE_ERROR_TYPE;
            break;
        }
        // fallthrough
    default:
        return lauxh_argerror(
            L, 2, "string, table, error or error_type expected, got %s",
            luaL_typename(L, 2));
    }

    lua_settop(L, 2);
    lua_insert(L, 1);
    goto *label;

COMPARE_ERROR_TYPE:
    if (err->ref_type != LUA_NOREF) {
        lauxh_pushref(L, err->ref_type);
        src = (uintptr_t)lua_touserdata(L, -1);
        lua_pop(L, 1);
        goto COMPARE;
    }

COMPARE_STRING:
    lauxh_pushref(L, err->ref_msg);
    src = (uintptr_t)lua_tostring(L, -1);
    lua_pop(L, 1);
    goto COMPARE;

COMPARE_TABLE:
    lauxh_pushref(L, err->ref_msg);
    src = (uintptr_t)lua_topointer(L, -1);
    lua_pop(L, 1);

COMPARE:
    if (src == target) {
        return 1;
    }
    // check wrapped error
    if (err->ref_wrap != LUA_NOREF) {
        lauxh_pushref(L, err->ref_wrap);
        err = lua_touserdata(L, -1);
        src = (uintptr_t)err;
        lua_replace(L, 2);
        goto *label;
    }

    // not found
    lua_pushnil(L);

    return 1;
}

static int typeof_lua(lua_State *L)
{
    if (lauxh_ismetatableof(L, 1, LE_ERROR_MT)) {
        le_error_t *err = lua_touserdata(L, 1);
        lauxh_pushref(L, err->ref_type);
    } else if (lauxh_ismetatableof(L, 1, LE_ERROR_TYPE_MT)) {
        lua_settop(L, 1);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

static int debug_lua(lua_State *L)
{
    lua_pushboolean(L, lauxh_checkboolean(L, 1));
    lua_setfield(L, LUA_REGISTRYINDEX, LE_ERROR_DEBUG_FLG);
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
LUALIB_API int le_open_error_errno(lua_State *L);
LUALIB_API int le_open_error_message(lua_State *L);

LUALIB_API int luaopen_error(lua_State *L)
{
    struct luaL_Reg mmethod[] = {
        {"__gc",       gc_lua      },
        {"__tostring", tostring_lua},
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
        {NULL,      NULL       }
    };

    // create metatable
    luaL_newmetatable(L, LE_ERROR_MT);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // metamethods
    for (struct luaL_Reg *ptr = mmethod; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    lua_pop(L, 1);

    // set default debug flag to false
    lua_pushboolean(L, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, LE_ERROR_DEBUG_FLG);

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
    lua_pushliteral(L, "errno");
    le_open_error_errno(L);
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
