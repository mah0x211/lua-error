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

#include <ctype.h>
#include <lauxhlib.h>
#include <lua.h>
#include <string.h>
#include <unistd.h>

#define LE_ERROR_MT          "error"
#define LE_ERROR_REGISTRY_MT "error.registry"
#define LE_ERROR_TYPE_MT     "error.type"
#define LE_ERROR_MESSAGE_MT  "error.message"

typedef struct {
    int ref_msg;
    int ref_op;
    lua_Integer code;
} le_error_message_t;

typedef struct {
    int ref_name;
    int ref_msg;
    lua_Integer code;
} le_error_type_t;

typedef struct {
    // runtime error
    int ref_msg;
    int ref_where;
    int ref_traceback;
    int ref_wrap;
    // error_type_t
    int ref_type;
} le_error_t;

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

static inline void le_loadlib(lua_State *L, int level)
{
    int top = lua_gettop(L);
    luaL_getmetatable(L, LE_ERROR_MT);
    if (lua_isnil(L, -1) &&
        ((luaL_loadstring(L, "require('error')") || lua_pcall(L, 0, 0, 0)))) {
        le_where(L, level);
        lua_insert(L, lua_gettop(L) - 1);
        lua_concat(L, 2);
        lua_error(L);
    }
    lua_settop(L, top);
}

#define LE_ERROR_DEBUG_FLG "error.debug"

static inline int le_isdebug(lua_State *L)
{
    int isdebug = 0;

    // get debug flag
    lua_getfield(L, LUA_REGISTRYINDEX, LE_ERROR_DEBUG_FLG);
    isdebug = !lua_isnoneornil(L, -1) && lauxh_checkboolean(L, -1);
    lua_pop(L, 1);

    return isdebug;
}

/**
 * create a new structured message that equivalent to the following code;
 *
 *  error.message.new(message [, op [, code]])
 *
 * push all arguments (including nil) onto the stack and call the API with
 * the msg index.
 * the last argument must be placed at the top of the stack.
 * this function removes all arguments from the stack.
 */
static inline int le_new_message_ex(lua_State *L, int msgidx, int default_code)
{
    int idx = (msgidx < 0) ? lua_gettop(L) + msgidx + 1 : msgidx;
    le_error_message_t *errm = NULL;
    const char *op           = NULL;
    lua_Integer code         = default_code;

    luaL_checkany(L, idx);
    op   = lauxh_optstring(L, idx + 1, NULL);
    code = lauxh_optinteger(L, idx + 2, code);

    // create message
    errm  = lua_newuserdata(L, sizeof(le_error_message_t));
    *errm = (le_error_message_t){
        .ref_msg = LUA_NOREF,
        .ref_op  = LUA_NOREF,
        .code    = code,
    };
    lauxh_setmetatable(L, LE_ERROR_MESSAGE_MT);
    errm->ref_msg = lauxh_refat(L, idx);
    if (op) {
        errm->ref_op = lauxh_refat(L, idx + 1);
    }
    // remove all arguments
    lua_replace(L, idx);
    lua_settop(L, idx);

    return 1;
}
#define le_new_message(L, msgidx) le_new_message_ex(L, msgidx, -1)

/**
 * create a new error that equivalent to the following code;
 *
 *  error.new(msg [, wrap [, level [, traceback]]])
 *
 * push all arguments (including nil) onto the stack and call the API with
 * the msg index.
 * the last argument must be placed at the top of the stack.
 * this function removes all arguments from the stack.
 */
static inline int le_new_error_ex(lua_State *L, int msgidx, int default_code)
{
    int idx          = (msgidx < 0) ? lua_gettop(L) + msgidx + 1 : msgidx;
    le_error_t *err  = NULL;
    le_error_t *wrap = lauxh_optudata(L, idx + 1, LE_ERROR_MT, NULL);
    int level        = (int)lauxh_optuint8(L, idx + 2, 1);
    int traceback    = lauxh_optboolean(L, idx + 3, 0);

    if (wrap) {
        lua_settop(L, idx + 1);
    }

    // check message argument
    luaL_checkany(L, idx);
    if (!lauxh_ismetatableof(L, idx, LE_ERROR_MESSAGE_MT)) {
        // convert message to error.message
        lua_pushvalue(L, idx);
        le_new_message_ex(L, -1, default_code);
        lua_replace(L, idx);
    }

    err  = lua_newuserdata(L, sizeof(le_error_t));
    *err = (le_error_t){
        .ref_msg       = LUA_NOREF,
        .ref_where     = LUA_NOREF,
        .ref_traceback = LUA_NOREF,
        .ref_wrap      = LUA_NOREF,
        .ref_type      = LUA_NOREF,
    };
    lauxh_setmetatable(L, LE_ERROR_MT);
    err->ref_msg = lauxh_refat(L, idx);
    // with wrap argument
    if (wrap) {
        err->ref_wrap = lauxh_refat(L, idx + 1);
    }
    le_where(L, level);
    err->ref_where = lauxh_ref(L);
    if (le_isdebug(L) || traceback) {
        lauxh_traceback(NULL, L, NULL, level);
        err->ref_traceback = lauxh_ref(L);
    }
    // remove all arguments
    lua_replace(L, idx);
    lua_settop(L, idx);

    return 1;
}
#define le_new_error(L, msgidx) le_new_error_ex(L, msgidx, -1)

/**
 * error type registry
 */
static inline void le_registry(lua_State *L)
{
    lua_getfield(L, LUA_REGISTRYINDEX, LE_ERROR_REGISTRY_MT ".REGISTRY_TABLE");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        // create new registry table as a weak reference table
        lua_newtable(L);
        luaL_newmetatable(L, LE_ERROR_REGISTRY_MT);
        lauxh_pushstr2tbl(L, "__mode", "v");
        lua_setmetatable(L, -2);
        // store it into LUA_REGISTRYINDEX
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX,
                     LE_ERROR_REGISTRY_MT ".REGISTRY_TABLE");
    }
}

/**
 * delete a error type from registry that equivalent to the following code;
 *
 *  error.type.del(name)
 */
static inline int le_registry_del(lua_State *L, const char *name)
{
    le_registry(L);
    lua_getfield(L, -1, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return 0;
    }
    lua_pop(L, 1);
    lua_pushnil(L);
    lua_setfield(L, -2, name);
    lua_pop(L, 1);

    return 1;
}

/**
 * get a error type object from registry that equivalent to the following code;
 *
 *  error.type.get(name)
 */
static inline int le_registry_get(lua_State *L, const char *name)
{
    le_registry(L);
    lua_getfield(L, -1, name);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return 0;
    } else if (!lauxh_ismetatableof(L, -1, LE_ERROR_TYPE_MT)) {
        lua_pushfstring(L,
                        "the error type registry is corrupted: "
                        "the registered error type '%s'"
                        "is not type of " LE_ERROR_TYPE_MT,
                        name);
        return lua_error(L);
    }

    // remove registry table
    lua_replace(L, -2);

    return 1;
}

/**
 * create a new error type that equivalent to the following code;
 *
 *  error.type.new(name [, code [, message]])
 *
 * push name arguments onto the stack and call the API with the name index.
 * the last argument must be placed at the top of the stack.
 * this function removes all arguments from the stack.
 */
static inline int le_new_type(lua_State *L, int nameidx)
{
    int idx = (nameidx < 0) ? lua_gettop(L) + nameidx + 1 : nameidx;
    le_error_type_t *errt = NULL;
    size_t len            = 0;
    const char *name      = lauxh_checklstring(L, idx, &len);
    lua_Integer code      = lauxh_optinteger(L, idx + 1, -1);
    const char *msg       = lauxh_optstring(L, idx + 2, NULL);

    // verify type name
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

    if (le_registry_get(L, name)) {
        // name already used in other error type
        return lauxh_argerror(L, idx, "already used in other error type");
    }

    // create new le_error_type_t
    errt           = lua_newuserdata(L, sizeof(le_error_type_t));
    errt->ref_name = lauxh_refat(L, idx);
    errt->code     = code;
    errt->ref_msg  = (msg) ? lauxh_refat(L, idx + 2) : LUA_NOREF;
    lauxh_setmetatable(L, LE_ERROR_TYPE_MT);
    // remove all arguments
    lua_replace(L, idx);
    lua_settop(L, idx);
    // register
    le_registry(L);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, name);
    lua_pop(L, 1);

    return 1;
}

/**
 * create a new typed error that equivalent to the following code;
 *
 *  <myerr>:new([message [, wrap [, level [, traceback]]]])
 *
 * push all arguments (including nil) onto the stack and call the API with
 * the type index.
 * the last argument must be placed at the top of the stack.
 * this function removes all arguments from the stack.
 */
static inline int le_new_typed_error(lua_State *L, int typeidx)
{
    int idx = (typeidx < 0) ? lua_gettop(L) + typeidx + 1 : typeidx;
    le_error_type_t *errt = luaL_checkudata(L, idx, LE_ERROR_TYPE_MT);
    int nomsg             = lua_isnoneornil(L, idx + 1);
    le_error_t *err       = NULL;

    if (nomsg) {
        lua_pushnil(L);
        if ((idx + 1) < lua_gettop(L)) {
            lua_replace(L, (idx + 1));
        }
    }
    le_new_error_ex(L, idx + 1, errt->code);
    err           = lua_touserdata(L, -1);
    err->ref_type = lauxh_refat(L, idx);
    // remove all arguments
    lua_replace(L, idx);
    lua_settop(L, idx);

    return 1;
}

static inline void le_tostring(lua_State *L, int idx)
{
    int type = lua_type(L, idx);

    if (idx < 0) {
        idx = lua_gettop(L) + idx + 1;
    }

    if (luaL_callmeta(L, idx, "__tostring")) {
        if (!lua_isstring(L, -1)) {
            luaL_error(L, "\"__tostring\" metamethod must return a string");
        }
    } else if (type != LUA_TSTRING) {
        switch (type) {
        case LUA_TNONE:
        case LUA_TNIL:
            lua_pushstring(L, "nil");
            break;

        case LUA_TNUMBER:
        case LUA_TBOOLEAN:
            lua_pushstring(L, lua_tostring(L, idx));
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
            break;
        }
        lua_replace(L, idx);
    }
}

#endif
