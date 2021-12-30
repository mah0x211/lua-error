# lua-error

[![test](https://github.com/mah0x211/lua-error/actions/workflows/test.yml/badge.svg)](https://github.com/mah0x211/lua-error/actions/workflows/test.yml)

additional features to the error module.

## Installation

```sh
luarocks install error
```

## Use from C module

the `error` module installs `lua_error.h` in the lua include directory.

the following API can be used to create an error object or error type object.


### static inline int le_new_error(lua_State *L, int msgidx)

create a new error that equivalent to `error.new` function.


### static inline int le_new_error_type(lua_State *L, int nameidx)

create a new error type that equivalent to `error.type.new(name)` function.


### static inline int le_new_type_error(lua_State *L, int typeidx)

create a new typed error that equivalent to `<myerr>:new(msg [, wrap [, level [, traceback]]])` method.


## Module Structure

```lua
local error = require('error')
```

`error` module contains the following functions and submodules;

- `__call` metamethod that equivalent to Lua's built-in `error` function
- functions of `error` module
- `error.type` submodule
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



## `error.type` module


### errt = error.type.new(name)

creates a new error type object.

**Params**

- `name:string`: name of the error type to use for stringification.
  - length must be between `1-127` characters.
  - first character must be an `a-zA-Z` character.
  - only the following characters can be used: `a-zA-Z0-9`, `.` and `_`.

**Returns**

- `errt:error.type`: error type object.


### name = errt:name()

`name()` method returns the name of the error type object.

**Returns**

- `name:string`: the name of the error type object.


### err = errt:new(message [, wrap [, level [, traceback]]])

equivalent to the `error.new` function, but sets the error type object to the error object.

**Returns**

- `err:error`: a new error object that holds the error type object.


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


