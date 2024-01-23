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

// lua
#include <lauxlib.h>
#include <lua.h>

static int where_lua(lua_State *L)
{
    int level     = (int)luaL_optinteger(L, 1, 1);
    lua_Debug ar  = {0};
    luaL_Buffer b = {0};

#if !defined(LUA_LJDIR) && LUA_VERSION_NUM < 502
    // NOTE: In Lua 5.1, stack level includes this c function
    level += 1;
#endif

    lua_settop(L, 1);
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
    return 1;
}

LUALIB_API int luaopen_error_where(lua_State *L)
{
    lua_pushcfunction(L, where_lua);
    return 1;
}
