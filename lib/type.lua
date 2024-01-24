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
local type = type
local find = string.find
local format = string.format
local tostring = tostring
local require = require
local argexpected = require('argexpected')
local instanceof = require('metamodule').instanceof
--- constants
local INF_POS = math.huge
local INF_NEG = -INF_POS

--- is_int returns true if the specified value is finite integer.
--- @param v any
--- @return boolean
local function is_int(v)
    return type(v) == 'number' and v >= INF_NEG and v <= INF_POS and v % 1 == 0
end

--- is_pint returns true if the specified value is finite positive integer.
--- @param v any
--- @return boolean
local function is_pint(v)
    return is_int(v) and v > 0
end

--- @type table<string, error.type>
local REGISTRY = setmetatable({}, {
    __mode = 'v',
})

--- @class error.type
--- @field name string
--- @field message string
--- @field code integer
local Type = {}

--- init creates a new error.type.
--- @param name string
--- @param code integer?
--- @param message string?
function Type:init(name, code, message)
    argexpected(type(name) == 'string', 1, 'string expected, got %s', type(name))
    argexpected(#name > 0 and #name <= 127, 1,
                'string length between 1-127 expected, got %d', #name)
    argexpected(find(name, '^%a[%w_.]*$') ~= nil, 1,
                'format is %q expected, got %q', '^%a[%w_.]*$', name)
    argexpected(code == nil or is_int(code), 2, 'integer expected, got %q', code)
    argexpected(message == nil or type(message) == 'string', 3,
                'string expected, got %s', type(message))
    if REGISTRY[name] then
        error(format('error.type %q already used in other error type', name), 2)
    end

    self.name = name
    self.code = code or -1
    self.message = message

    -- register to registry
    REGISTRY[self.name] = self

    -- override _STRING field
    self._STRING = format('%s: %s', name, tostring(self))

    return self
end

--- new creates a new error.type instance.
--- @param message any
--- @param wrap error?
--- @param level integer?
--- @param trace boolean?
--- @return error err
function Type:new(message, wrap, level, trace)
    argexpected(wrap == nil or instanceof(wrap, 'error'), 2,
                'error expected, got %s', type(wrap))
    argexpected(level == nil or is_pint(level), 3,
                'positive integer expected, got %q', level)
    argexpected(trace == nil or type(trace) == 'boolean', 4,
                'boolean expected, got %s', type(trace))
    level = 1 + (level or 1)

    local new_error = require('error').new
    local err = new_error(message, wrap, level, trace)
    err.type = self
    err.code = self.code
    return err
end

--- del deletes the specified error.type from registry.
--- @param name string
--- @return boolean
local function del(name)
    argexpected(type(name) == 'string', 1, 'string expected, got %q', type(name))
    if REGISTRY[name] then
        REGISTRY[name] = nil
        return true
    end
    return false
end

--- get returns the specified error.type from registry.
--- @param name string
--- @return error.type?
local function get(name)
    argexpected(type(name) == 'string', 1, 'string expected, got %s', type(name))
    return REGISTRY[name]
end

return {
    get = get,
    del = del,
    new = require('metamodule').new(Type),
}
