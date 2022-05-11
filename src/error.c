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
    le_error_t *err = luaL_checkudata(L, 1, LE_ERROR_MT);
    int idx         = luaL_checkoption(L, 2, NULL, fields);

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
            code = ((le_error_type_t *)lua_touserdata(L, -1))->code;
            lua_pop(L, 1);
        }
        lua_pushinteger(L, code);
        return 1;
    }

    case 3:
        if (err->ref_msg != LUA_NOREF) {
            lauxh_pushref(L, err->ref_msg);
            lauxh_pushref(
                L, ((le_error_message_t *)lua_touserdata(L, -1))->ref_op);
            return 1;
        }
    }
    lua_pushnil(L);
    return 1;
}

static int tostring_lua(lua_State *L)
{
    le_error_t *err          = luaL_checkudata(L, 1, LE_ERROR_MT);
    le_error_message_t *errm = NULL;
    le_error_type_t *errt    = NULL;
    int ref_typemsg          = LUA_NOREF;
    luaL_Buffer b            = {0};

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
        le_tostring(L, -1);
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
        le_tostring(L, -1);
        luaL_addvalue(&b);
    } else {
        // <type.message>
        lauxh_pushref(L, ref_typemsg);
        le_tostring(L, -1);
        luaL_addvalue(&b);
        if (errm->ref_msg != LUA_REFNIL) {
            // use a message as sub-message as follows:
            //  <type.message> (<message>)
            luaL_addstring(&b, " (");
            lauxh_pushref(L, errm->ref_msg);
            le_tostring(L, -1);
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
    le_error_t *err = lua_touserdata(L, 1);

    err->ref_msg       = lauxh_unref(L, err->ref_msg);
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
    int is_msg = 0;

    lua_settop(L, 1);
    if (lua_isnoneornil(L, 1)) {
        lua_pushnil(L);
    } else if (lauxh_isuserdataof(L, 1, LE_ERROR_MT)) {
        lauxh_pushref(L, ((le_error_t *)lua_touserdata(L, 1))->ref_msg);
        is_msg = 1;
    } else {
        is_msg = lauxh_isuserdataof(L, 1, LE_ERROR_MESSAGE_MT);
    }

    lua_pushboolean(L, is_msg);
    return 2;
}

static int is_lua(lua_State *L)
{
    le_error_t *err          = NULL;
    le_error_message_t *errm = NULL;
    size_t len               = 0;
    const char *target       = NULL;
    void *label              = &&CMP_MESSAGE;

    // do nothing if target is nil or first argument is not an error object
    if (lua_isnoneornil(L, 2) || !lauxh_isuserdataof(L, 1, LE_ERROR_MT)) {
        lua_pushnil(L);
        return 1;
    }
    lua_settop(L, 2);
    lua_insert(L, 1);
    err = (le_error_t *)lua_touserdata(L, -1);

    // check target argument
    switch (lua_type(L, 1)) {
    case LUA_TSTRING:
        target = lua_tolstring(L, 1, &len);
        label  = &&CMP_STRING;
        break;

    case LUA_TUSERDATA:
        if (lauxh_isuserdataof(L, 1, LE_ERROR_MT)) {
            label = &&CMP_ERROR;
        } else if (lauxh_isuserdataof(L, 1, LE_ERROR_TYPE_MT)) {
            // compare with error.type
            label = &&CMP_ERROR_TYPE;
        } else if (lauxh_isuserdataof(L, 1, LE_ERROR_MESSAGE_MT)) {
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

    // set metatable
    lua_newtable(L);
    // lock metatable
    lauxh_pushnum2tbl(L, "__metatable", -1);
    // export a call metamethod that equivalent to built-in error function
    lauxh_pushfn2tbl(L, "__call", call_lua);
    lua_setmetatable(L, -2);

    return 1;
}
