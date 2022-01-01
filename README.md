# lua-error

[![test](https://github.com/mah0x211/lua-error/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-error/actions/workflows/test.yml)

additional features to the error module.

## Installation

```sh
luarocks install error
```

## Usage

```lua
local error = require('error')
local LF = '\n\n'

print('-- raises an error')
local ok, msg = pcall(error, 'raises an error')
assert(not ok)
print(msg, LF)

print('-- create new error type object')
local errt = error.type.new('example_error')
print(errt, LF)

print('-- create new error object')
local msg_str = 'my new error object'
local err1 = error.new(msg_str)
print(err1, LF)

print('-- extract a message of error')
print(error.cause(err1), LF)

print('-- create new typed error object that wraps err1')
local msg_tbl = {
    message = 'this typed error wraps err1',
    tostring = function(self, where)
        return where .. self.message
    end,
}
local err2 = errt:new(msg_tbl, err1)
print(err2, LF)

print('-- get the wrapped error object')
print(error.unwrap(err2), LF)

print('-- create new error object with level and traceback arguments')
local msg_mtbl = setmetatable({
    message = 'my new error at stack level 2 with stack traceback',
}, {
    __tostring = function(self, where, traceback)
        return where .. self.message .. '\n' .. traceback
    end,
})
local function nest3(err)
    return error.new(msg_mtbl, err, 2, true)
end

local function nest2(err)
    local newerr = nest3(err)
    return newerr
end

local function tail_call(err)
    return nest2(err)
end

local function nest1(err)
    local newerr = tail_call(err)
    return newerr
end

local err3 = nest1(err2)
print(err3, LF)

print('-- create new error object that wraps err1, err2 and err3')
local last_err = error.new('this is the last error', err3)
print(last_err, LF)

print('-- get the err1 by message from the last_err')
print(error.is(last_err, msg_str), LF)

print('-- get the err2 that matches error type errt')
print(error.is(last_err, errt), LF)

print('-- it returns nil if message does not matches with any of error message')
print(error.is(last_err, 'unknown message'), LF)
```

the above code will be output as follows;

```
-- raises an error
raises an error	


-- create new error type object
example_error: 0x7fe5005088e8	


-- create new error object
./example.lua:15: in main chunk: my new error object	


-- extract a message of error
my new error object	


-- create new typed error object that wraps err1
./example.lua:28: in main chunk: this typed error wraps err1
./example.lua:15: in main chunk: my new error object	


-- get the wrapped error object
./example.lua:15: in main chunk: my new error object	


-- create new error object with level and traceback arguments
./example.lua:47: in function <./example.lua:46>: my new error at stack level 2 with stack traceback
stack traceback:
	./example.lua:47: in function <./example.lua:46>
	(...tail calls...)
	./example.lua:56: in local 'nest1'
	./example.lua:60: in main chunk
	[C]: in ?
./example.lua:28: in main chunk: this typed error wraps err1
./example.lua:15: in main chunk: my new error object	


-- create new error object that wraps err1, err2 and err3
./example.lua:64: in main chunk: this is the last error
./example.lua:47: in function <./example.lua:46>: my new error at stack level 2 with stack traceback
stack traceback:
	./example.lua:47: in function <./example.lua:46>
	(...tail calls...)
	./example.lua:56: in local 'nest1'
	./example.lua:60: in main chunk
	[C]: in ?
./example.lua:28: in main chunk: this typed error wraps err1
./example.lua:15: in main chunk: my new error object	


-- get the err1 by message from the last_err
./example.lua:15: in main chunk: my new error object	


-- get the err2 that matches error type errt
./example.lua:28: in main chunk: this typed error wraps err1
./example.lua:15: in main chunk: my new error object	


-- it returns nil if message does not matches with any of error message
nil	

```

## Use from C module

the `error` module installs `lua_error.h` in the lua include directory.

the following API can be used to create an error object or error type object.

### static inline void le_loadlib(lua_State *L, int level)

load the lua-error module.  
**NOTE:** you must call this API at least once before using the following API.


### static inline int le_new_error(lua_State *L, int msgidx)

create a new error that equivalent to `error.new(message [, wrap [, level [, traceback]]])` function.


### static inline int le_new_error_type(lua_State *L, int nameidx)

create a new error type that equivalent to `error.type.new(name)` function.


### static inline int le_new_type_error(lua_State *L, int typeidx)

create a new typed error that equivalent to `<myerr>:new(msg [, wrap [, level [, traceback]]])` method.


### static inline int le_registry_get(lua_State *L, const char *name)

get a error type object from registry that equivalent to `error.type.get(name)` function.


### static inline int le_registry_del(lua_State *L, const char *name)

delete a error type from registry that equivalent to `error.type.del(name)` function.


## Module Structure

```lua
local error = require('error')
```

`error` module contains the following functions and submodules;

- `__call` metamethod that equivalent to Lua's built-in `error` function
- functions of `error` module
- `error.type` submodule
- `error.errno` submodule
- `error.check` submodule


## `error` module

### error(message [, level])

calls the `__call` metamethod, which is equivalent to Lua's built-in function `error`.


### err = error.new(message [, wrap [, level [, traceback]]])

creates a new error object.

**Params**

- `message:string|table`: string message, or structured message as table.  
  - `structured message` must has a function in the `tostring` field or `__tostring` metamethod field.
- `wrap:error`: `nil` or other error objects to be included in the new error object.
- `level:integer`: specifies how to get the error position. (default: `1`)
- `traceback:boolean`: get the stack traceback and keep it in the error object. (default: `false`)

**Returns**

- `err:error`: error object that contains the `__tostring` metamethod.


### err = error.toerror(message [, wrap [, level [, traceback]]])

equivalent to the `error.new` function, but if the `message` is an `error` object, it will be returned as is.


### err = error.unwrap(err)

return a wrapped error object.

**Params**

- `err:error`: error object.

**Returns**

- `err:error`: a wrapped error object or `nil`.


### message = error.cause(err)

return a error message.

**Params**

- `err:error`: error object.

**Returns**

- `message:string|table`: error message.


### err = error.is(err, target)

extract the error object that **strictly matches** the `target` from the error object.  
the target object is converted to `uintptr_t` and compared with the error object or internal value.

**Params**

- `err:error`: error object.
- `target:error|error.type|string|table`: `error` object, `error.type` object, or error message.

**Returns**

- `err:error`: matched error object, or `nil`.


### errt = error.typeof(err)

get the error type object associated with the error object.

**Params**

- `err:error`: error object.

**Returns**

- `errt:error.type`: error type object, or `nil`.


## `error.type` module


### errt = error.type.new(name)

creates a new error type object.  
the created error type object will be kept in `the registry table` that cannot be accessed directly.  

**NOTE:** the registry table is set up as a weak reference table `__mode = 'v'`.

**Params**

- `name:string`: name of the error type to use for stringification.
  - length must be between `1-127` characters.
  - first character must be an `a-zA-Z` character.
  - only the following characters can be used: `a-zA-Z0-9`, `.` and `_`.

**Returns**

- `errt:error.type`: error type object.


### errt = error.type.get(name)

get a error type object from the registry table.

**Params**

- `name:string`: name of the error type.

**Returns**

- `errt:error.type`: error type object.


### ok = error.type.del(name)

delete a error type from the registry table.

**Params**

- `name:string`: name of the error type.

**Returns**

- `ok:boolean`: `true` on success.


### name = errt:name()

`name()` method returns the name of the error type object.

**Returns**

- `name:string`: the name of the error type object.


### err = errt:new(message [, wrap [, level [, traceback]]])

equivalent to the `error.new` function, but sets the error type object to the error object.

**Returns**

- `err:error`: a new error object that holds the error type object.


## `error.errno` module

this module exports the error type of the error number `errno` defined in the running system.  
the list of error numbers is defined in [var/errno.txt](var/errno.txt).

```lua
local dump = require('dump')
local errno = require('error').errno
print(dump(errno))
--
-- the above code will be output as follows;
--
-- {
--     E2BIG = "E2BIG: 0x7facfd40ce88",
--     EACCES = "EACCES: 0x7facfd40cf28",
--     EADDRINUSE = "EADDRINUSE: 0x7facfd40c838",
--     EADDRNOTAVAIL = "EADDRNOTAVAIL: 0x7facfd40cec8",
--     EAFNOSUPPORT = "EAFNOSUPPORT: 0x7facfd40d058",
--     EAGAIN = "EAGAIN: 0x7facfd40cf98",
--     EALREADY = "EALREADY: 0x7facfd40d2c8",
--     EAUTH = "EAUTH: 0x7facfd40d308",
--     EBADARCH = "EBADARCH: 0x7facfd40d378",
--     EBADEXEC = "EBADEXEC: 0x7facfd40c438",
--     EBADF = "EBADF: 0x7facfd40c478",
--     EBADMACHO = "EBADMACHO: 0x7facfd40b918",
--     EBADMSG = "EBADMSG: 0x7facfd40c4b8",
--     EBADRPC = "EBADRPC: 0x7facfd40c518",
--     EBUSY = "EBUSY: 0x7facfd40c578",
--     ECANCELED = "ECANCELED: 0x7facfd40d098",
--     ECHILD = "ECHILD: 0x7facfd40d0f8",
--     ECONNABORTED = "ECONNABORTED: 0x7facfd40d168",
--     ECONNREFUSED = "ECONNREFUSED: 0x7facfd40d1d8",
--     ECONNRESET = "ECONNRESET: 0x7facfd40d248",
--     EDEADLK = "EDEADLK: 0x7facfd40d558",
--     EDESTADDRREQ = "EDESTADDRREQ: 0x7facfd40d5c8",
--     EDEVERR = "EDEVERR: 0x7facfd40d628",
--     EDOM = "EDOM: 0x7facfd40d688",
--     EDQUOT = "EDQUOT: 0x7facfd40d6e8",
--     EEXIST = "EEXIST: 0x7facfd40d748",
--     EFAULT = "EFAULT: 0x7facfd40d7a8",
--     EFBIG = "EFBIG: 0x7facfd40d808",
--     EFTYPE = "EFTYPE: 0x7facfd40d868",
--     EHOSTDOWN = "EHOSTDOWN: 0x7facfd40d8d8",
--     EHOSTUNREACH = "EHOSTUNREACH: 0x7facfd40e168",
--     EIDRM = "EIDRM: 0x7facfd40e1a8",
--     EILSEQ = "EILSEQ: 0x7facfd40e208",
--     EINPROGRESS = "EINPROGRESS: 0x7facfd40d968",
--     EINTR = "EINTR: 0x7facfd40d9c8",
--     EINVAL = "EINVAL: 0x7facfd40d3d8",
--     EIO = "EIO: 0x7facfd40d438",
--     EISCONN = "EISCONN: 0x7facfd40d498",
--     EISDIR = "EISDIR: 0x7facfd40d4f8",
--     ELAST = "ELAST: 0x7facfd40dba8",
--     ELOOP = "ELOOP: 0x7facfd40dc08",
--     EMFILE = "EMFILE: 0x7facfd40dc68",
--     EMLINK = "EMLINK: 0x7facfd40dcc8",
--     EMSGSIZE = "EMSGSIZE: 0x7facfd40dd38",
--     EMULTIHOP = "EMULTIHOP: 0x7facfd40dda8",
--     ENAMETOOLONG = "ENAMETOOLONG: 0x7facfd40de18",
--     ENEEDAUTH = "ENEEDAUTH: 0x7facfd40de88",
--     ENETDOWN = "ENETDOWN: 0x7facfd40def8",
--     ENETRESET = "ENETRESET: 0x7facfd40df68",
--     ENETUNREACH = "ENETUNREACH: 0x7facfd40dfd8",
--     ENFILE = "ENFILE: 0x7facfd40e038",
--     ENOATTR = "ENOATTR: 0x7facfd40e098",
--     ENOBUFS = "ENOBUFS: 0x7facfd40e0f8",
--     ENODATA = "ENODATA: 0x7facfd40e268",
--     ENODEV = "ENODEV: 0x7facfd40e2c8",
--     ENOENT = "ENOENT: 0x7facfd40e328",
--     ENOEXEC = "ENOEXEC: 0x7facfd40e388",
--     ENOLCK = "ENOLCK: 0x7facfd40e3e8",
--     ENOLINK = "ENOLINK: 0x7facfd40e448",
--     ENOMEM = "ENOMEM: 0x7facfd40e4a8",
--     ENOMSG = "ENOMSG: 0x7facfd40e508",
--     ENOPROTOOPT = "ENOPROTOOPT: 0x7facfd40e578",
--     ENOSPC = "ENOSPC: 0x7facfd40e5d8",
--     ENOSR = "ENOSR: 0x7facfd40e638",
--     ENOSTR = "ENOSTR: 0x7facfd40e698",
--     ENOSYS = "ENOSYS: 0x7facfd40e6f8",
--     ENOTBLK = "ENOTBLK: 0x7facfd40e738",
--     ENOTCONN = "ENOTCONN: 0x7facfd40da08",
--     ENOTDIR = "ENOTDIR: 0x7facfd40da68",
--     ENOTEMPTY = "ENOTEMPTY: 0x7facfd40dad8",
--     ENOTRECOVERABLE = "ENOTRECOVERABLE: 0x7facfd40db48",
--     ENOTSOCK = "ENOTSOCK: 0x7facfd40e928",
--     ENOTSUP = "ENOTSUP: 0x7facfd40e988",
--     ENOTTY = "ENOTTY: 0x7facfd40e9e8",
--     ENXIO = "ENXIO: 0x7facfd40ea48",
--     EOPNOTSUPP = "EOPNOTSUPP: 0x7facfd40eab8",
--     EOVERFLOW = "EOVERFLOW: 0x7facfd40eb28",
--     EOWNERDEAD = "EOWNERDEAD: 0x7facfd40eb98",
--     EPERM = "EPERM: 0x7facfd40ebf8",
--     EPFNOSUPPORT = "EPFNOSUPPORT: 0x7facfd40ec68",
--     EPIPE = "EPIPE: 0x7facfd40ecc8",
--     EPROCLIM = "EPROCLIM: 0x7facfd40ed38",
--     EPROCUNAVAIL = "EPROCUNAVAIL: 0x7facfd40eda8",
--     EPROGMISMATCH = "EPROGMISMATCH: 0x7facfd40ee18",
--     EPROGUNAVAIL = "EPROGUNAVAIL: 0x7facfd40ee88",
--     EPROTO = "EPROTO: 0x7facfd40eee8",
--     EPROTONOSUPPORT = "EPROTONOSUPPORT: 0x7facfd40ef58",
--     EPROTOTYPE = "EPROTOTYPE: 0x7facfd40efc8",
--     EPWROFF = "EPWROFF: 0x7facfd40f028",
--     ERANGE = "ERANGE: 0x7facfd40f088",
--     EREMOTE = "EREMOTE: 0x7facfd40f0e8",
--     EROFS = "EROFS: 0x7facfd40f148",
--     ERPCMISMATCH = "ERPCMISMATCH: 0x7facfd40f1b8",
--     ESHLIBVERS = "ESHLIBVERS: 0x7facfd40f228",
--     ESHUTDOWN = "ESHUTDOWN: 0x7facfd40f298",
--     ESOCKTNOSUPPORT = "ESOCKTNOSUPPORT: 0x7facfd40f308",
--     ESPIPE = "ESPIPE: 0x7facfd40f368",
--     ESRCH = "ESRCH: 0x7facfd40f3c8",
--     ESTALE = "ESTALE: 0x7facfd40f428",
--     ETIME = "ETIME: 0x7facfd40f488",
--     ETIMEDOUT = "ETIMEDOUT: 0x7facfd40f4f8",
--     ETOOMANYREFS = "ETOOMANYREFS: 0x7facfd40f568",
--     ETXTBSY = "ETXTBSY: 0x7facfd40f5c8",
--     EUSERS = "EUSERS: 0x7facfd40f628",
--     EWOULDBLOCK = "EWOULDBLOCK: 0x7facfd40f698",
--     EXDEV = "EXDEV: 0x7facfd40f6f8"
-- }
```

## `error.check` module

The following functions checks the type of the first argument, and raises an error if the type is not correct.

### error.check.file(v)

checks whether a `v` is `file` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.noneornil(v)

checks whether a `v` is `none` or `nil`, or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.boolean(v)

checks whether a `v` is `boolean` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.pointer(v)

checks whether a `v` is `pointer (light userdata)` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.string(v)

checks whether a `v` is `string` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.table(v)

checks whether a `v` is `table` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.func(v)

checks whether a `v` is `function` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.userdata(v)

checks whether a `v` is `userdata` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.thread(v)

checks whether a `v` is `thread` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.number(v)

checks whether a `v` is `number` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.finite(v)

checks whether a `v` is `finite number` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.unsigned(v)

checks whether a `v` is `unsigned number` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.int(v)

checks whether a `v` is `integer` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.int8(v)

checks whether a `v` is `int8` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.int16(v)

checks whether a `v` is `int16` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.int32(v)

checks whether a `v` is `int32` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.int64(v)

checks whether a `v` is `int64` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.uint(v)

checks whether a `v` is `uint` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.uint8(v)

checks whether a `v` is `uint8` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.uint16(v)

checks whether a `v` is `uint16` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.uint32(v)

checks whether a `v` is `uint32` or not.

**Parameters**

- `v:any`: value to be tested.

### error.check.uint64(v)

checks whether a `v` is `uint64` or not.

**Parameters**

- `v:any`: value to be tested.


