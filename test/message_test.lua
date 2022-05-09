local testcase = require('testcase')
local error = require('error')
local message = error.message

function testcase.new()
    -- test that create new structured message
    local msg = message.new('hello')
    assert.equal(msg.message, 'hello')
    assert.is_nil(msg.op)
    assert.is_nil(msg.code)
    assert.match(tostring(msg), 'hello')

    -- test that create with op
    msg = message.new('hello', 'test-op')
    assert.equal(msg.op, 'test-op')
    assert.match(tostring(msg), '[op:test-op] hello')

    -- test that create with code
    msg = message.new('hello', 'test-op', 123)
    assert.equal(msg.code, 123)
    assert.match(tostring(msg), '[op:test-op][code:123] hello')

    -- test that create by table
    local tbl = {}
    msg = message.new(tbl, 'test-op', 123)
    assert.equal(msg.code, 123)
    assert.match(tostring(msg), '[op:test-op][code:123] table: ')

    -- test that calls tbl.__tostring metamethod
    setmetatable(tbl, {
        __tostring = function()
            return '__tostring metamethod'
        end,
    })
    assert.match(tostring(msg), '[op:test-op][code:123] __tostring metamethod')
end

function testcase.with_error_type()
    error.debug(true)
    local t = error.type.new('myerr')

    -- test that create new structured message from string message
    local err = t:new(message.new('hello'))
    assert.match(err, '[type:myerr] hello')

    -- test that create new structured message from table message
    err = t:new(message.new({
        'hello',
    }))
    assert.match(err, '[type:myerr] table: ')

    -- test that create new structured message from table message that contains __tostring metamethod
    err = t:new(message.new(setmetatable({
        message = 'hello',
    }, {
        __tostring = function(self)
            return self.message
        end,
    })))
    assert.match(err, '[type:myerr] hello')

    -- test that throws an error if __tostring metamethod not returned string
    err = t:new(message.new(setmetatable({}, {
        __tostring = function()
        end,
    })))
    assert.match(assert.throws(tostring, err),
                 '"__tostring" metamethod must return a string')

end
