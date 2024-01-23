--
-- Copyright (C) 2024 Masatoshi Fukunaga
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to deal
-- in the Software without restriction, including without limitation the rights
-- to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
-- copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
--
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
-- OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
-- THE SOFTWARE.
--
--- assign to local
local error = error
local type = type
local match = string.match
local concat = table.concat
local traceback = debug.traceback
local argexpected = require('argexpected')
local format = require('string.format')
local instanceof = require('metamodule').instanceof
local tostring = require('error.tostring')
local new_message = require('error.message').new
local getwhere = require('error.where')
--- constants
local INF_POS = math.huge
local DEBUG_MODE = false

--- is_pint returns true if the specified value is finite positive integer.
--- @param v any
--- @return boolean
local function is_pint(v)
    return type(v) == 'number' and v >= 1 and v <= INF_POS and v % 1 == 0
end

--- @class error
--- @field code integer
--- @field message error.message
--- @field op string? operation name that copied from message.op
--- @field where string
--- @field wrap error?
--- @field traceback string?
--- @field type error.type?
local Error = {}

--- __tostring metamethod
--- @return string
function Error:__tostring()
    local msgs = {
        self.where,
    }
    local errtype = self.type
    local typemsg = errtype and errtype.message

    -- [type.name:type.code]
    if errtype then
        msgs[#msgs + 1] =
            '[' .. errtype.name .. ':' .. tostring(errtype.code) .. ']'
    end

    -- [message.op]
    if self.op then
        msgs[#msgs + 1] = '[' .. self.op .. ']'
    end

    if #msgs > 1 then
        -- append space
        msgs[#msgs + 1] = ' '
    end

    if not typemsg then
        -- <message.message>
        local msg = '(no message)'
        if self.message.message then
            msg = tostring(self.message.message)
        end
        msgs[#msgs + 1] = msg
    else
        -- <type.message>
        msgs[#msgs + 1] = typemsg
        if self.message.message then
            -- use a message as sub-message as follows:
            --  <type.message> (<message>)
            msgs[#msgs + 1] = ' (' .. tostring(self.message.message) .. ')'
        end
    end

    if self.traceback then
        msgs[#msgs + 1] = '\n'
        msgs[#msgs + 1] = self.traceback
    end

    if self.wrap then
        -- convert wrapped error to string
        msgs[#msgs + 1] = '\n'
        msgs[#msgs + 1] = tostring(self.wrap)
    end

    return concat(msgs)
end

--- init creates a new error.
--- @param message any
--- @param werr error?
--- @param level integer?
--- @param trace boolean?
--- @return error
function Error:init(message, werr, level, trace)
    argexpected(werr == nil or instanceof(werr, 'error'), 2,
                'error expected, got %q', werr)
    argexpected(level == nil or is_pint(level), 3,
                'positive integer expected, got %q', level)
    argexpected(trace == nil or type(trace) == 'boolean', 4,
                'boolean expected, got %q', trace)
    level = 1 + (level or 1)

    self.code = -1
    if instanceof(message, 'error.message') then
        self.message = message --[[@as error.message]]
    else
        -- convert message to error.message
        self.message = new_message(message)
    end

    self.op = self.message.op
    self.wrap = werr
    self.where = getwhere(level)
    if DEBUG_MODE or trace then
        -- NOTE: In luajit, the debug.traceback() will return the nil if the
        -- message argument is nil. So, we need to remove the first empty line
        -- of the traceback string.
        self.traceback = match(traceback('', level), '%s*(.+)$')
    end

    return self
end

Error = require('metamodule').new(Error)

--- create a new error with the specified message.
--- @param ... any
--- @return error err
local function errorf(...)
    local s, unused = format(...)
    if not unused then
        return Error(s, nil, 2)
    elseif not instanceof(unused[1], 'error') then
        -- convert non-error object to string and append to message
        local v = unused[1]
        s = s .. ': ' .. (type(v) == 'string' and v or tostring(v))
        unused[1] = nil
    end

    return Error(s, unused[1], unused[2], unused[3])
end

--- toerror converts the specified value to error.
--- @param v any
--- @param werr error?
--- @param level integer?
--- @param trace boolean?
--- @return error
local function toerror(v, werr, level, trace)
    if instanceof(v, 'error') then
        return v
    elseif level == nil then
        level = 2
    elseif type(level) == 'number' then
        level = 1 + level
    end

    -- NOTE: prevent to tail call optimization to keep the correct stack trace
    local err = Error(v, werr, level, trace)
    return err
end

--- unwrap returns the wrapped error.
--- @return error?
local function unwrap(err)
    argexpected(instanceof(err, 'error'), 1, 'error expected, got %s', type(err))
    return err.wrap
end

--- cause returns the root cause of the error.
--- @param err error?
local function cause(err)
    if err == nil then
        return nil, false
    elseif instanceof(err, 'error') then
        return err.message, true
    end
    return err, instanceof(err, 'error.message')
end

--- typeof returns the error.type of error.
--- @param err error
--- @return error.type?
local function typeof(err)
    if not err or type(err) ~= 'table' then
        return nil
    elseif instanceof(err, 'error') then
        return err.type
    elseif instanceof(err, 'error.type') then
        return err --[[@as error.type]]
    end
end

--- debug enables/disables debug mode.
--- @param enabled boolean
local function debug(enabled)
    argexpected(type(enabled) == 'boolean', 1, 'boolean expected, got %s',
                type(enabled))
    DEBUG_MODE = enabled
end

--- __call metamethod calls builtin error function.
--- @param _ table
--- @param msg string
--- @param level integer?
local function __call(_, msg, level)
    if type(msg) ~= 'string' then
        msg = tostring(msg)
    end

    if type(level) == 'number' and level > 0 then
        level = level + 1
    end
    error(msg, level)
end

return setmetatable({
    format = errorf,
    toerror = toerror,
    unwrap = unwrap,
    cause = cause,
    typeof = typeof,
    debug = debug,
    new = Error,
    check = require('error.check'),
    fatalf = require('error.fatalf'),
    is = require('error.is'),
    message = require('error.message'),
    type = require('error.type'),
}, {
    __metatable = -1,
    __call = __call,
})
