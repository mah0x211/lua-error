local testcase = require('testcase')
local error = require('error')
local message = error.message

function testcase.new()
    -- test that create new structured message
    local msg = message.new('hello')
    assert.equal(msg.message, 'hello')
    assert.is_nil(msg.op)
    assert.match(tostring(msg), 'hello')

    -- test that create with op
    msg = message.new('hello', 'test-op')
    assert.equal(msg.op, 'test-op')
    assert.match(tostring(msg), '[op:test-op] hello')

    -- test that create by table
    local tbl = {}
    msg = message.new(tbl, 'test-op')
    assert.match(tostring(msg), '[op:test-op] table: ')

    -- test that calls tbl.__tostring metamethod
    setmetatable(tbl, {
        __tostring = function()
            return '__tostring metamethod'
        end,
    })
    assert.match(tostring(msg), '[op:test-op] __tostring metamethod')

    -- test that throws an error if no argument
    local err = assert.throws(message.new)
    assert.match(err, 'value expected')
end

function testcase.with_error_type()
    error.debug(true)
    local t = error.type.new('myerr')

    -- test that create new structured message from string message
    local err = t:new(message.new('hello', 'myop'))
    assert.match(err, '[myerr:-1][myop] hello')
    assert.equal(err.op, 'myop')

    -- test that create new structured message from table message
    err = t:new(message.new({
        'hello',
    }))
    assert.match(err, '[myerr:-1] table: ')

    -- test that create new structured message from table message that contains __tostring metamethod
    err = t:new(message.new(setmetatable({
        message = 'hello',
    }, {
        __tostring = function(self)
            return self.message
        end,
    })))
    assert.match(err, '[myerr:-1] hello')

    -- test that throws an error if __tostring metamethod not returned string
    err = t:new(message.new(setmetatable({}, {
        __tostring = function()
        end,
    })))
    assert.match(assert.throws(tostring, err),
                 '"__tostring" metamethod must return a string')

end

function testcase.is()
    error.debug(true)
    local t = error.type.new('is_myerr')
    local msg = message.new('hello')
    local err = error.new('last-error', t:new(msg))

    -- test that get typed-error by error.message
    local terr = assert(error.is(err, msg))
    assert.equal(terr.message, msg)
end
