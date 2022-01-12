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

    msg = message.new('hello', 'test-op')
    assert.equal(msg.op, 'test-op')
    assert.match(tostring(msg), '[op:test-op] hello')

    msg = message.new('hello', 'test-op', 123)
    assert.equal(msg.code, 123)
    assert.match(tostring(msg), '[op:test-op][code:123] hello')
end

