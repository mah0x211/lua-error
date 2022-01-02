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


### static inline int le_new_type(lua_State *L, int nameidx)

create a new error type that equivalent to `error.type.new(name)` function.


### static inline int le_new_typed_error(lua_State *L, int typeidx)

create a new typed error that equivalent to `<myerr>:new(msg [, wrap [, level [, traceback]]])` method.


### static inline int le_registry_get(lua_State *L, const char *name)

get a error type object from registry that equivalent to `error.type.get(name)` function.

if an error type object found, its return `1`,  otherwise return `0`.


### static inline int le_registry_del(lua_State *L, const char *name)

delete a error type from registry that equivalent to `error.type.del(name)` function.

if an error type object has deleted, its return `1`,  otherwise return `0`.


### static inline int le_errno2error_type(lua_State *L, int err)

get an error type object from `error.errno` table that equivalent to `error.errno[err]`.

if an error type object found, its return `1`,  otherwise return `0`.


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
--     [1] = "EPERM: 0x7f8dcfc57388",
--     [2] = "ENOENT: 0x7f8dcfc56dc8",
--     [3] = "ESRCH: 0x7f8dcfc57848",
--     [4] = "EINTR: 0x7f8dcfc55278",
--     [5] = "EIO: 0x7f8dcfc552f8",
--     [6] = "ENXIO: 0x7f8dcfc57288",
--     [7] = "E2BIG: 0x7f8dcfc52598",
--     [8] = "ENOEXEC: 0x7f8dcfc56e08",
--     [9] = "EBADF: 0x7f8dcfc52428",
--     [10] = "ECHILD: 0x7f8dcfc55478",
--     [11] = "EDEADLK: 0x7f8dcfc542f8",
--     [12] = "ENOMEM: 0x7f8dcfc56ec8",
--     [13] = "EACCES: 0x7f8dcfc52648",
--     [14] = "EFAULT: 0x7f8dcfc55578",
--     [15] = "ENOTBLK: 0x7f8dcfc550f8",
--     [16] = "EBUSY: 0x7f8dcfc54c38",
--     [17] = "EEXIST: 0x7f8dcfc55538",
--     [18] = "EXDEV: 0x7f8dcfc557a8",
--     [19] = "ENODEV: 0x7f8dcfc56d88",
--     [20] = "ENOTDIR: 0x7f8dcfc55178",
--     [21] = "EISDIR: 0x7f8dcfc55378",
--     [22] = "EINVAL: 0x7f8dcfc552b8",
--     [23] = "ENFILE: 0x7f8dcfc54eb8",
--     [24] = "EMFILE: 0x7f8dcfc55438",
--     [25] = "ENOTTY: 0x7f8dcfc57248",
--     [26] = "ETXTBSY: 0x7f8dcfc556e8",
--     [27] = "EFBIG: 0x7f8dcfc555b8",
--     [28] = "ENOSPC: 0x7f8dcfc56f88",
--     [29] = "ESPIPE: 0x7f8dcfc57808",
--     [30] = "EROFS: 0x7f8dcfc576c8",
--     [31] = "EMLINK: 0x7f8dcfc54cb8",
--     [32] = "EPIPE: 0x7f8dcfc57408",
--     [33] = "EDOM: 0x7f8dcfc554b8",
--     [34] = "ERANGE: 0x7f8dcfc57648",
--     [35] = "EWOULDBLOCK: 0x7f8dcfc55768",
--     [36] = "EINPROGRESS: 0x7f8dcfc55238",
--     [37] = "EALREADY: 0x7f8dcfc543f8",
--     [38] = "ENOTSOCK: 0x7f8dcfc571c8",
--     [39] = "EDESTADDRREQ: 0x7f8dcfc54338",
--     [40] = "EMSGSIZE: 0x7f8dcfc54cf8",
--     [41] = "EPROTOTYPE: 0x7f8dcfc575c8",
--     [42] = "ENOPROTOOPT: 0x7f8dcfc56f48",
--     [43] = "EPROTONOSUPPORT: 0x7f8dcfc57588",
--     [44] = "ESOCKTNOSUPPORT: 0x7f8dcfc577c8",
--     [45] = "ENOTSUP: 0x7f8dcfc57208",
--     [46] = "EPFNOSUPPORT: 0x7f8dcfc573c8",
--     [47] = "EAFNOSUPPORT: 0x7f8dcfc54538",
--     [48] = "EADDRINUSE: 0x7f8dcfc525d8",
--     [49] = "EADDRNOTAVAIL: 0x7f8dcfc52468",
--     [50] = "ENETDOWN: 0x7f8dcfc54df8",
--     [51] = "ENETUNREACH: 0x7f8dcfc54e78",
--     [52] = "ENETRESET: 0x7f8dcfc54e38",
--     [53] = "ECONNABORTED: 0x7f8dcfc54238",
--     [54] = "ECONNRESET: 0x7f8dcfc542b8",
--     [55] = "ENOBUFS: 0x7f8dcfc54f38",
--     [56] = "EISCONN: 0x7f8dcfc55338",
--     [57] = "ENOTCONN: 0x7f8dcfc55138",
--     [58] = "ESHUTDOWN: 0x7f8dcfc57788",
--     [59] = "ETOOMANYREFS: 0x7f8dcfc556a8",
--     [60] = "ETIMEDOUT: 0x7f8dcfc55668",
--     [61] = "ECONNREFUSED: 0x7f8dcfc54278",
--     [62] = "ELOOP: 0x7f8dcfc553f8",
--     [63] = "ENAMETOOLONG: 0x7f8dcfc54d78",
--     [64] = "EHOSTDOWN: 0x7f8dcfc54fb8",
--     [65] = "EHOSTUNREACH: 0x7f8dcfc54ff8",
--     [66] = "ENOTEMPTY: 0x7f8dcfc551b8",
--     [67] = "EPROCLIM: 0x7f8dcfc57448",
--     [68] = "EUSERS: 0x7f8dcfc55728",
--     [69] = "EDQUOT: 0x7f8dcfc554f8",
--     [70] = "ESTALE: 0x7f8dcfc57888",
--     [71] = "EREMOTE: 0x7f8dcfc57688",
--     [72] = "EBADRPC: 0x7f8dcfc54bf8",
--     [73] = "ERPCMISMATCH: 0x7f8dcfc57708",
--     [74] = "EPROGUNAVAIL: 0x7f8dcfc57508",
--     [75] = "EPROGMISMATCH: 0x7f8dcfc574c8",
--     [76] = "EPROCUNAVAIL: 0x7f8dcfc57488",
--     [77] = "ENOLCK: 0x7f8dcfc56e48",
--     [78] = "ENOSYS: 0x7f8dcfc550b8",
--     [79] = "EFTYPE: 0x7f8dcfc555f8",
--     [80] = "EAUTH: 0x7f8dcfc54438",
--     [81] = "ENEEDAUTH: 0x7f8dcfc54db8",
--     [82] = "EPWROFF: 0x7f8dcfc57608",
--     [83] = "EDEVERR: 0x7f8dcfc54378",
--     [84] = "EOVERFLOW: 0x7f8dcfc57308",
--     [85] = "EBADEXEC: 0x7f8dcfc523e8",
--     [86] = "EBADARCH: 0x7f8dcfc54478",
--     [87] = "ESHLIBVERS: 0x7f8dcfc57748",
--     [88] = "EBADMACHO: 0x7f8dcfc544b8",
--     [89] = "ECANCELED: 0x7f8dcfc54c78",
--     [90] = "EIDRM: 0x7f8dcfc55038",
--     [91] = "ENOMSG: 0x7f8dcfc56f08",
--     [92] = "EILSEQ: 0x7f8dcfc551f8",
--     [93] = "ENOATTR: 0x7f8dcfc54ef8",
--     [94] = "EBADMSG: 0x7f8dcfc544f8",
--     [95] = "EMULTIHOP: 0x7f8dcfc54d38",
--     [96] = "ENODATA: 0x7f8dcfc54f78",
--     [97] = "ENOLINK: 0x7f8dcfc56e88",
--     [98] = "ENOSR: 0x7f8dcfc56fc8",
--     [99] = "ENOSTR: 0x7f8dcfc55078",
--     [100] = "EPROTO: 0x7f8dcfc57548",
--     [101] = "ETIME: 0x7f8dcfc578c8",
--     [102] = "EOPNOTSUPP: 0x7f8dcfc572c8",
--     [104] = "ENOTRECOVERABLE: 0x7f8dcfc57188",
--     [105] = "EOWNERDEAD: 0x7f8dcfc57348",
--     [106] = "ELAST: 0x7f8dcfc553b8",
--     E2BIG = "E2BIG: 0x7f8dcfc52598",
--     EACCES = "EACCES: 0x7f8dcfc52648",
--     EADDRINUSE = "EADDRINUSE: 0x7f8dcfc525d8",
--     EADDRNOTAVAIL = "EADDRNOTAVAIL: 0x7f8dcfc52468",
--     EAFNOSUPPORT = "EAFNOSUPPORT: 0x7f8dcfc54538",
--     EAGAIN = "EAGAIN: 0x7f8dcfc543b8",
--     EALREADY = "EALREADY: 0x7f8dcfc543f8",
--     EAUTH = "EAUTH: 0x7f8dcfc54438",
--     EBADARCH = "EBADARCH: 0x7f8dcfc54478",
--     EBADEXEC = "EBADEXEC: 0x7f8dcfc523e8",
--     EBADF = "EBADF: 0x7f8dcfc52428",
--     EBADMACHO = "EBADMACHO: 0x7f8dcfc544b8",
--     EBADMSG = "EBADMSG: 0x7f8dcfc544f8",
--     EBADRPC = "EBADRPC: 0x7f8dcfc54bf8",
--     EBUSY = "EBUSY: 0x7f8dcfc54c38",
--     ECANCELED = "ECANCELED: 0x7f8dcfc54c78",
--     ECHILD = "ECHILD: 0x7f8dcfc55478",
--     ECONNABORTED = "ECONNABORTED: 0x7f8dcfc54238",
--     ECONNREFUSED = "ECONNREFUSED: 0x7f8dcfc54278",
--     ECONNRESET = "ECONNRESET: 0x7f8dcfc542b8",
--     EDEADLK = "EDEADLK: 0x7f8dcfc542f8",
--     EDESTADDRREQ = "EDESTADDRREQ: 0x7f8dcfc54338",
--     EDEVERR = "EDEVERR: 0x7f8dcfc54378",
--     EDOM = "EDOM: 0x7f8dcfc554b8",
--     EDQUOT = "EDQUOT: 0x7f8dcfc554f8",
--     EEXIST = "EEXIST: 0x7f8dcfc55538",
--     EFAULT = "EFAULT: 0x7f8dcfc55578",
--     EFBIG = "EFBIG: 0x7f8dcfc555b8",
--     EFTYPE = "EFTYPE: 0x7f8dcfc555f8",
--     EHOSTDOWN = "EHOSTDOWN: 0x7f8dcfc54fb8",
--     EHOSTUNREACH = "EHOSTUNREACH: 0x7f8dcfc54ff8",
--     EIDRM = "EIDRM: 0x7f8dcfc55038",
--     EILSEQ = "EILSEQ: 0x7f8dcfc551f8",
--     EINPROGRESS = "EINPROGRESS: 0x7f8dcfc55238",
--     EINTR = "EINTR: 0x7f8dcfc55278",
--     EINVAL = "EINVAL: 0x7f8dcfc552b8",
--     EIO = "EIO: 0x7f8dcfc552f8",
--     EISCONN = "EISCONN: 0x7f8dcfc55338",
--     EISDIR = "EISDIR: 0x7f8dcfc55378",
--     ELAST = "ELAST: 0x7f8dcfc553b8",
--     ELOOP = "ELOOP: 0x7f8dcfc553f8",
--     EMFILE = "EMFILE: 0x7f8dcfc55438",
--     EMLINK = "EMLINK: 0x7f8dcfc54cb8",
--     EMSGSIZE = "EMSGSIZE: 0x7f8dcfc54cf8",
--     EMULTIHOP = "EMULTIHOP: 0x7f8dcfc54d38",
--     ENAMETOOLONG = "ENAMETOOLONG: 0x7f8dcfc54d78",
--     ENEEDAUTH = "ENEEDAUTH: 0x7f8dcfc54db8",
--     ENETDOWN = "ENETDOWN: 0x7f8dcfc54df8",
--     ENETRESET = "ENETRESET: 0x7f8dcfc54e38",
--     ENETUNREACH = "ENETUNREACH: 0x7f8dcfc54e78",
--     ENFILE = "ENFILE: 0x7f8dcfc54eb8",
--     ENOATTR = "ENOATTR: 0x7f8dcfc54ef8",
--     ENOBUFS = "ENOBUFS: 0x7f8dcfc54f38",
--     ENODATA = "ENODATA: 0x7f8dcfc54f78",
--     ENODEV = "ENODEV: 0x7f8dcfc56d88",
--     ENOENT = "ENOENT: 0x7f8dcfc56dc8",
--     ENOEXEC = "ENOEXEC: 0x7f8dcfc56e08",
--     ENOLCK = "ENOLCK: 0x7f8dcfc56e48",
--     ENOLINK = "ENOLINK: 0x7f8dcfc56e88",
--     ENOMEM = "ENOMEM: 0x7f8dcfc56ec8",
--     ENOMSG = "ENOMSG: 0x7f8dcfc56f08",
--     ENOPROTOOPT = "ENOPROTOOPT: 0x7f8dcfc56f48",
--     ENOSPC = "ENOSPC: 0x7f8dcfc56f88",
--     ENOSR = "ENOSR: 0x7f8dcfc56fc8",
--     ENOSTR = "ENOSTR: 0x7f8dcfc55078",
--     ENOSYS = "ENOSYS: 0x7f8dcfc550b8",
--     ENOTBLK = "ENOTBLK: 0x7f8dcfc550f8",
--     ENOTCONN = "ENOTCONN: 0x7f8dcfc55138",
--     ENOTDIR = "ENOTDIR: 0x7f8dcfc55178",
--     ENOTEMPTY = "ENOTEMPTY: 0x7f8dcfc551b8",
--     ENOTRECOVERABLE = "ENOTRECOVERABLE: 0x7f8dcfc57188",
--     ENOTSOCK = "ENOTSOCK: 0x7f8dcfc571c8",
--     ENOTSUP = "ENOTSUP: 0x7f8dcfc57208",
--     ENOTTY = "ENOTTY: 0x7f8dcfc57248",
--     ENXIO = "ENXIO: 0x7f8dcfc57288",
--     EOPNOTSUPP = "EOPNOTSUPP: 0x7f8dcfc572c8",
--     EOVERFLOW = "EOVERFLOW: 0x7f8dcfc57308",
--     EOWNERDEAD = "EOWNERDEAD: 0x7f8dcfc57348",
--     EPERM = "EPERM: 0x7f8dcfc57388",
--     EPFNOSUPPORT = "EPFNOSUPPORT: 0x7f8dcfc573c8",
--     EPIPE = "EPIPE: 0x7f8dcfc57408",
--     EPROCLIM = "EPROCLIM: 0x7f8dcfc57448",
--     EPROCUNAVAIL = "EPROCUNAVAIL: 0x7f8dcfc57488",
--     EPROGMISMATCH = "EPROGMISMATCH: 0x7f8dcfc574c8",
--     EPROGUNAVAIL = "EPROGUNAVAIL: 0x7f8dcfc57508",
--     EPROTO = "EPROTO: 0x7f8dcfc57548",
--     EPROTONOSUPPORT = "EPROTONOSUPPORT: 0x7f8dcfc57588",
--     EPROTOTYPE = "EPROTOTYPE: 0x7f8dcfc575c8",
--     EPWROFF = "EPWROFF: 0x7f8dcfc57608",
--     ERANGE = "ERANGE: 0x7f8dcfc57648",
--     EREMOTE = "EREMOTE: 0x7f8dcfc57688",
--     EROFS = "EROFS: 0x7f8dcfc576c8",
--     ERPCMISMATCH = "ERPCMISMATCH: 0x7f8dcfc57708",
--     ESHLIBVERS = "ESHLIBVERS: 0x7f8dcfc57748",
--     ESHUTDOWN = "ESHUTDOWN: 0x7f8dcfc57788",
--     ESOCKTNOSUPPORT = "ESOCKTNOSUPPORT: 0x7f8dcfc577c8",
--     ESPIPE = "ESPIPE: 0x7f8dcfc57808",
--     ESRCH = "ESRCH: 0x7f8dcfc57848",
--     ESTALE = "ESTALE: 0x7f8dcfc57888",
--     ETIME = "ETIME: 0x7f8dcfc578c8",
--     ETIMEDOUT = "ETIMEDOUT: 0x7f8dcfc55668",
--     ETOOMANYREFS = "ETOOMANYREFS: 0x7f8dcfc556a8",
--     ETXTBSY = "ETXTBSY: 0x7f8dcfc556e8",
--     EUSERS = "EUSERS: 0x7f8dcfc55728",
--     EWOULDBLOCK = "EWOULDBLOCK: 0x7f8dcfc55768",
--     EXDEV = "EXDEV: 0x7f8dcfc557a8"
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


