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
local err2 = errt:new('this typed error wraps err1', err1)
print(err2, LF)

print('-- get the wrapped error object')
print(error.unwrap(err2), LF)

local function nest3(err)
    print('-- create new error object with level and traceback arguments')
    return error.new(setmetatable({
        message = 'my new error from __tostring metamethod',
    }, {
        __tostring = function(self)
            return self.message
        end,
    }), err, 2, true)
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

