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
#include <math.h>
#include <stdint.h>

static inline const char *typename(lua_State *L, int type)
{
    return (type == LUA_TLIGHTUSERDATA) ? "pointer" : lua_typename(L, type);
}

static inline int isint(lua_State *L, int idx)
{
#if LUA_VERSION_NUM >= 503
    return lua_isinteger(L, idx);
#else
    return (lua_type(L, idx) == LUA_TNUMBER && isfinite(lua_tonumber(L, idx)) &&
            lua_tonumber(L, idx) == (lua_Number)lua_tointeger(L, idx));
#endif
}

static void argerror(lua_State *L, const char *exp, const char *act)
{
    int idx       = (int)lauxh_optuint8(L, 2, 1);
    int level     = (int)lauxh_optuint8(L, 3, 1);
    int traceback = (int)lauxh_optboolean(L, 4, 1);
    lua_Debug ar;
    luaL_Buffer b;

    luaL_buffinit(L, &b);
    luaL_where(L, level);
    luaL_addvalue(&b);
    lua_pushfstring(L, "bad argument #%d ", idx);
    luaL_addvalue(&b);

    if (!lua_getstack(L, level, &ar)) {
        // no stack frame
        lua_pushfstring(L, "(%s expected, got %s)", exp, act);
        luaL_addvalue(&b);
    } else {
        lua_getinfo(L, "n", &ar);
        if (!ar.name) {
            ar.name = "?";
        }
        lua_pushfstring(L, "to '%s' ", ar.name);
        luaL_addvalue(&b);
        lua_pushfstring(L, "(%s expected, got %s)", exp, act);
        luaL_addvalue(&b);
    }

    if (traceback) {
        luaL_addchar(&b, '\n');
        lauxh_traceback(NULL, L, NULL, level);
        luaL_addvalue(&b);
    }

    luaL_pushresult(&b);
    lua_error(L);
}

static void checkarg(lua_State *L)
{
    // check(v [, argidx [, level [, traceback]]])
    lauxh_optuint8(L, 2, 1);
    lauxh_optuint8(L, 3, 1);
    lauxh_optboolean(L, 4, 1);
}

static void checktype_ex(lua_State *L, int type, const char *exp)
{
    int t = lua_type(L, 1);
    checkarg(L);

    if (t != type) {
        argerror(L, exp, typename(L, t));
    }
}

#define checktype(L, t) checktype_ex(L, t, lua_typename(L, t))

static lua_Integer checkint(lua_State *L, const char *exp)
{
    checkarg(L);
    if (!isint(L, 1)) {
        int type = lua_type(L, 1);

        if (type == LUA_TNUMBER) {
            lua_Number n = lua_tonumber(L, 1);
            if (isinf(n) || isnan(n)) {
                argerror(L, exp, lua_tostring(L, 1));
            }
        }
        argerror(L, exp, typename(L, type));
    }

    return lua_tointeger(L, 1);
}

#define tokenize2(x, y)    x##y
#define tokenize3(x, y, z) tokenize2(x##y, z)
#define stringer(x)        #x

#define check_pintsize(L, size)                                                \
 do {                                                                          \
  lua_Integer v = checkint(L, "pint" stringer(size));                          \
  if (v < 1 || (uint64_t)v > tokenize3(UINT, size, _MAX)) {                    \
   argerror(L, "pint" stringer(size), "an out of range value");                \
  }                                                                            \
 } while (0)

static int pint16_lua(lua_State *L)
{
    check_pintsize(L, 16);
    return 0;
}

static int pint8_lua(lua_State *L)
{
    check_pintsize(L, 8);
    return 0;
}

static int pint_lua(lua_State *L)
{
    if (checkint(L, "pint") < 1) {
        argerror(L, "pint", "an out of range value");
    }
    return 0;
}

#define check_uintsize(L, size)                                                \
 do {                                                                          \
  lua_Integer v = checkint(L, "uint" stringer(size));                          \
  if (v < 0 || (uint64_t)v > tokenize3(UINT, size, _MAX)) {                    \
   argerror(L, "uint" stringer(size), "an out of range value");                \
  }                                                                            \
 } while (0)

static int uint64_lua(lua_State *L)
{
    check_uintsize(L, 64);
    return 0;
}

static int uint32_lua(lua_State *L)
{
    check_uintsize(L, 32);
    return 0;
}

static int uint16_lua(lua_State *L)
{
    check_uintsize(L, 16);
    return 0;
}

static int uint8_lua(lua_State *L)
{
    check_uintsize(L, 8);
    return 0;
}

static int uint_lua(lua_State *L)
{
    if (checkint(L, "uint") < 0) {
        argerror(L, "uint", "an out of range value");
    }
    return 0;
}

#define check_intsize(L, size)                                                 \
 do {                                                                          \
  int64_t v = (int64_t)checkint(L, "int" stringer(size));                      \
  if (v < tokenize3(INT, size, _MIN) || v > tokenize3(INT, size, _MAX)) {      \
   argerror(L, "int" stringer(size), "an out of range value");                 \
  }                                                                            \
 } while (0)

static int int64_lua(lua_State *L)
{
    check_intsize(L, 64);
    return 0;
}

static int int32_lua(lua_State *L)
{
    check_intsize(L, 32);
    return 0;
}

static int int16_lua(lua_State *L)
{
    check_intsize(L, 16);
    return 0;
}

static int int8_lua(lua_State *L)
{
    check_intsize(L, 8);
    return 0;
}

static int int_lua(lua_State *L)
{
    checkint(L, "int");
    return 0;
}

static int unsigned_lua(lua_State *L)
{
    lua_Number n = 0;

    checktype_ex(L, LUA_TNUMBER, "unsigned number");
    n = lua_tonumber(L, 1);
    if (isnan(n)) {
        argerror(L, "unsigned number", lua_tostring(L, 1));
    } else if (n < 0) {
        argerror(L, "unsigned number", "signed number");
    }
    return 0;
}

static int finite_lua(lua_State *L)
{
    checktype_ex(L, LUA_TNUMBER, "finite number");
    if (!isfinite(lua_tonumber(L, 1))) {
        argerror(L, "finite number", lua_tostring(L, 1));
    }
    return 0;
}

static int number_lua(lua_State *L)
{
    checktype(L, LUA_TNUMBER);
    if (isnan(lua_tonumber(L, 1))) {
        argerror(L, "number", lua_tostring(L, 1));
    }
    return 0;
}

static int thread_lua(lua_State *L)
{
    checktype(L, LUA_TTHREAD);
    return 0;
}

static int userdata_lua(lua_State *L)
{
    checktype(L, LUA_TUSERDATA);
    return 0;
}

static int function_lua(lua_State *L)
{
    checktype(L, LUA_TFUNCTION);
    return 0;
}

static int table_lua(lua_State *L)
{
    checktype(L, LUA_TTABLE);
    return 0;
}

static int string_lua(lua_State *L)
{
    checktype(L, LUA_TSTRING);
    return 0;
}

static int pointer_lua(lua_State *L)
{
    checktype_ex(L, LUA_TLIGHTUSERDATA, "pointer");
    return 0;
}

static int boolean_lua(lua_State *L)
{
    checktype(L, LUA_TBOOLEAN);
    return 0;
}

static int noneornil_lua(lua_State *L)
{
    if (!lua_isnoneornil(L, 1)) {
        argerror(L, "none or nil", typename(L, lua_type(L, 1)));
    }
    return 0;
}

static int file_lua(lua_State *L)
{
    if (!lauxh_isuserdataof(L, 1, LUA_FILEHANDLE)) {
        argerror(L, "FILE*", lua_typename(L, lua_type(L, 1)));
    }
    return 0;
}

LUALIB_API int le_open_error_check(lua_State *L)
{
    struct luaL_Reg funcs[] = {
        {"file",      file_lua     },
        {"noneornil", noneornil_lua},
        {"boolean",   boolean_lua  },
        {"pointer",   pointer_lua  },
        {"string",    string_lua   },
        {"table",     table_lua    },
        {"func",      function_lua },
        {"userdata",  userdata_lua },
        {"thread",    thread_lua   },
        {"number",    number_lua   },
        {"finite",    finite_lua   },
        {"unsigned",  unsigned_lua },
        {"pint",      pint_lua     },
        {"pint8",     pint8_lua    },
        {"pint16",    pint16_lua   },
        {"int",       int_lua      },
        {"int8",      int8_lua     },
        {"int16",     int16_lua    },
        {"int32",     int32_lua    },
        {"int64",     int64_lua    },
        {"uint",      uint_lua     },
        {"uint8",     uint8_lua    },
        {"uint16",    uint16_lua   },
        {"uint32",    uint32_lua   },
        {"uint64",    uint64_lua   },
        {NULL,        NULL         }
    };

    // export funcs
    lua_newtable(L);
    for (struct luaL_Reg *ptr = funcs; ptr->name; ptr++) {
        lauxh_pushfn2tbl(L, ptr->name, ptr->func);
    }

    return 1;
}
