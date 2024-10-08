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
local select = select
local type = type
local instanceof = require('metamodule').instanceof
local get_error_type = require('error.type').get

--- is_match_error_message returns true if err.message matches the target.
--- @param err error
--- @param target error.message
--- @return boolean ok
local function is_match_error_message(err, target)
    return err.message == target
end

--- is_match_error_type returns true if err.type matches the target.
--- @param err error
--- @param target error.type
--- @return boolean ok
local function is_match_error_type(err, target)
    return err.type == target
end

--- is_match_error returns true if err matches the target.
--- @param err error
--- @param target error
--- @return boolean ok
local function is_match_error(err, target)
    return err == target
end

--- is_match_message returns true if err.message.message matches the target.
--- @param err error
--- @param target any
--- @return boolean ok
local function is_match_message(err, target)
    return err.message.message == target
end

--- is_error extract the error that strictly matches the specified target from the error chain.
--- @param err error
--- @param target string|error|error.type|error.message
--- @return error?
local function is_error(err, target)
    if type(target) == 'string' then
        -- determine the error type from the target string
        local errtype = get_error_type(target)
        if errtype then
            target = errtype
        end
    end

    local t = type(target)
    local is_match = is_match_message
    if t == 'table' then
        if instanceof(target, 'error') then
            is_match = is_match_error
        elseif instanceof(target, 'error.type') then
            is_match = is_match_error_type
        elseif instanceof(target, 'error.message') then
            is_match = is_match_error_message
        end
    end

    repeat
        if is_match(err, target) then
            return err
        end
        -- unwrap
        err = err.wrap
    until err == nil
end

--- is extract the error that strictly matches the specified targets from the error chain.
--- @param err error
--- @param ... string|error|error.type|error.message
--- @return error?
local function is(err, ...)
    if not instanceof(err, 'error') then
        return nil
    end

    local n = select('#', ...)
    local targets = {
        ...,
    }
    for i = 1, n do
        local target = targets[i]
        local matched = target and is_error(err, target)
        if matched then
            return matched
        end
    end
end

return is
