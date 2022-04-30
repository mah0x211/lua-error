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

static int new_lua(lua_State *L)
{
    return le_new_typed_error(L, 1);
}

static int message_lua(lua_State *L)
{
    le_error_type_t *errt = luaL_checkudata(L, 1, LE_ERROR_TYPE_MT);

    lua_settop(L, 1);
    lauxh_pushref(L, errt->ref_msg);

    return 1;
}

static int code_lua(lua_State *L)
{
    le_error_type_t *errt = luaL_checkudata(L, 1, LE_ERROR_TYPE_MT);

    lua_settop(L, 1);
    lua_pushinteger(L, errt->code);

    return 1;
}

static int name_lua(lua_State *L)
{
    le_error_type_t *errt = luaL_checkudata(L, 1, LE_ERROR_TYPE_MT);

    lua_settop(L, 1);
    lauxh_pushref(L, errt->ref_name);

    return 1;
}

static int tostring_lua(lua_State *L)
{
    le_error_type_t *errt = luaL_checkudata(L, 1, LE_ERROR_TYPE_MT);

    lua_settop(L, 1);
    lauxh_pushref(L, errt->ref_name);
    lua_pushfstring(L, ": %p", errt);
    lua_concat(L, 2);

    return 1;
}

static int gc_lua(lua_State *L)
{
    le_error_type_t *errt = lua_touserdata(L, 1);

    errt->ref_name = lauxh_unref(L, errt->ref_name);

    return 0;
}

static int new_type_lua(lua_State *L)
{
    return le_new_type(L, 1);
}

static int del_lua(lua_State *L)
{
    const char *name = lauxh_checkstring(L, 1);

    lua_settop(L, 1);
    lua_pushboolean(L, le_registry_del(L, name));

    return 1;
}

static int get_lua(lua_State *L)
{
    const char *name = lauxh_checkstring(L, 1);

    lua_settop(L, 1);
    if (!le_registry_get(L, name)) {
        lua_pushnil(L);
    }

    return 1;
}

LUALIB_API int le_open_error_type(lua_State *L)
{
    struct luaL_Reg mmethod[] = {
        {"__gc",       gc_lua      },
        {"__tostring", tostring_lua},
        {NULL,         NULL        }
    };
    struct luaL_Reg method[] = {
        {"name",    name_lua   },
        {"code",    code_lua   },
        {"message", message_lua},
        {"new",     new_lua    },
        {NULL,      NULL       }
    };
    struct luaL_Reg funcs[] = {
        {"get", get_lua     },
        {"del", del_lua     },
        {"new", new_type_lua},
        {NULL,  NULL        }
    };

    // create metatable
    luaL_newmetatable(L, LE_ERROR_TYPE_MT);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // metamethods
    for (struct luaL_Reg *ptr = mmethod; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    // methods
    lua_pushstring(L, "__index");
    lua_newtable(L);
    for (struct luaL_Reg *ptr = method; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    lua_rawset(L, -3);
    lua_pop(L, 1);

    // export funcs
    lua_newtable(L);
    for (struct luaL_Reg *ptr = funcs; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }

    return 1;
}
