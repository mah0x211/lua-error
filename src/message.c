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

static inline void tostring(lua_State *L, int idx)
{
    int type = lua_type(L, idx);

    if (idx < 0) {
        idx = lua_gettop(L) + idx + 1;
    }

    if (luaL_callmeta(L, idx, "__tostring")) {
        if (!lua_isstring(L, -1)) {
            luaL_error(L, "'__tostring' must return a string");
        }
        lua_replace(L, idx);
    } else {
        switch (type) {
        case LUA_TSTRING:
            break;

        case LUA_TNIL:
        case LUA_TNUMBER:
        case LUA_TBOOLEAN:
            lua_pushstring(L, lua_tostring(L, idx));
            lua_replace(L, idx);
            break;

        // case LUA_TTHREAD:
        // case LUA_TLIGHTUSERDATA:
        // case LUA_TTABLE:
        // case LUA_TFUNCTION:
        // case LUA_TUSERDATA:
        // case LUA_TTHREAD:
        default:
            lua_pushfstring(L, "%s: %p", lua_typename(L, type),
                            lua_topointer(L, idx));
            lua_replace(L, idx);
            break;
        }
    }
}

//  tostring(self [, where [, traceback [, type]]])
static int tostring_lua(lua_State *L)
{
    int add_space = 0;
    luaL_Buffer b;

    lua_settop(L, 4);
    luaL_buffinit(L, &b);

    // whare
    if (!lua_isnil(L, 2)) {
        tostring(L, 2);
        lua_pushvalue(L, 2);
        luaL_addvalue(&b);
    }

    // type
    if (!lua_isnil(L, 4)) {
        le_error_type_t *t = luaL_checkudata(L, 4, LE_ERROR_TYPE_MT);
        // tostring as '[<type.name>] '
        lua_pushliteral(L, "[type:");
        luaL_addvalue(&b);
        lauxh_pushref(L, t->ref_name);
        luaL_addvalue(&b);
        luaL_addchar(&b, ']');
        add_space = 1;
    }

    // add '<op>'
    lua_getfield(L, 1, "op");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
    } else {
        lua_pushliteral(L, "[op:");
        luaL_addvalue(&b);
        tostring(L, -1);
        luaL_addvalue(&b);
        luaL_addchar(&b, ']');
        add_space = 1;
    }

    // add '<code>'
    lua_getfield(L, 1, "code");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
    } else {
        lua_pushliteral(L, "[code:");
        luaL_addvalue(&b);
        tostring(L, -1);
        luaL_addvalue(&b);
        luaL_addchar(&b, ']');
        add_space = 1;
    }

    // add '<message>'
    if (add_space) {
        luaL_addchar(&b, ' ');
    }
    lua_getfield(L, 1, "message");
    tostring(L, -1);
    luaL_addvalue(&b);

    // traceback
    if (!lua_isnil(L, 3)) {
        luaL_addchar(&b, '\n');
        tostring(L, 3);
        lua_pushvalue(L, 3);
        luaL_addvalue(&b);
    }

    luaL_pushresult(&b);

    return 1;
}

static int new_lua(lua_State *L)
{
    return le_new_message(L, 1);
}

LUALIB_API int le_open_error_message(lua_State *L)
{
    struct luaL_Reg mmethod[] = {
        {"__tostring", tostring_lua},
        {NULL,         NULL        }
    };
    struct luaL_Reg funcs[] = {
        {"new", new_lua},
        {NULL,  NULL   }
    };

    // create metatable
    luaL_newmetatable(L, LE_ERROR_MESSAGE_MT);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // metamethods
    for (struct luaL_Reg *ptr = mmethod; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }
    lua_pop(L, 1);

    // export funcs
    lua_newtable(L);
    for (struct luaL_Reg *ptr = funcs; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }

    return 1;
}
