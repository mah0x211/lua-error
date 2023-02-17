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
        "message",
        "op",
        NULL,
    };
    lua_error_message_t *errm = luaL_checkudata(L, 1, LUA_ERROR_MESSAGE_MT);
    int idx                   = luaL_checkoption(L, 2, NULL, fields);

    switch (idx) {
    case 0:
        lauxh_pushref(L, errm->ref_msg);
        break;

    default:
        lauxh_pushref(L, errm->ref_op);
        break;
    }

    return 1;
}

//  tostring(self [, where [, traceback [, type]]])
static int tostring_lua(lua_State *L)
{
    lua_error_message_t *errm = luaL_checkudata(L, 1, LUA_ERROR_MESSAGE_MT);
    luaL_Buffer b             = {0};

    lua_settop(L, 1);
    luaL_buffinit(L, &b);

    // add '<op>'
    if (errm->ref_op != LUA_NOREF) {
        lua_pushliteral(L, "[op:");
        luaL_addvalue(&b);
        lauxh_pushref(L, errm->ref_op);
        luaL_addvalue(&b);
        luaL_addstring(&b, "] ");
    }

    // add '<message>'
    lauxh_pushref(L, errm->ref_msg);
    le_tostring(L, -1);
    luaL_addvalue(&b);

    luaL_pushresult(&b);

    return 1;
}

static int gc_lua(lua_State *L)
{
    lua_error_message_t *errm = luaL_checkudata(L, 1, LUA_ERROR_MESSAGE_MT);

    errm->ref_msg = lauxh_unref(L, errm->ref_msg);
    errm->ref_op  = lauxh_unref(L, errm->ref_op);

    return 0;
}

static int new_lua(lua_State *L)
{
    return le_new_message(L, 1);
}

LUALIB_API int le_open_error_message(lua_State *L)
{
    struct luaL_Reg mmethod[] = {
        {"__tostring", tostring_lua},
        {"__gc",       gc_lua      },
        {"__index",    index_lua   },
        {NULL,         NULL        }
    };

    // create metatable
    luaL_newmetatable(L, LUA_ERROR_MESSAGE_MT);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // metamethods
    for (struct luaL_Reg *ptr = mmethod; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    lua_pop(L, 1);

    // export funcs
    lua_newtable(L);
    lauxh_pushfn2tbl(L, "new", new_lua);

    return 1;
}
