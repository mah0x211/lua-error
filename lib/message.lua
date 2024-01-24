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
local argexpected = require('argexpected')
local format = require('string.format')
local tostring = require('error.tostring')

--- @class error.message
--- @field message any
--- @field op string?
local Message = {}

--- __tostring metamethod
--- @return string
function Message:__tostring()
    if self.op then
        return format('[op:%s] %s', self.op, self.message)
    end
    return tostring(self.message)
end

--- init creates a new error.message.
--- @param message any
--- @param op string?
--- @return error.message
function Message:init(message, op)
    argexpected(op == nil or type(op) == 'string', 2, 'string expected, got %s',
                type(op))
    self.message = message
    self.op = op
    return self
end

return {
    new = require('metamodule').new(Message),
}
