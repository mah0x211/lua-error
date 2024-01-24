require('luacov')
local _, tester = assert(pcall(function()
    package.path = './?.lua;./test/?.lua;' .. package.path
    return require('./tester')
end))
local testcase = tester()
local builtin_error = error
local unpack = require('unpack')
local assert = require('assert')
local error = require('error')

function testcase.after_each()
    error.debug(false)
end

function testcase.after_all()
    error.debug(false)
end

function testcase.check_submodules()
    -- test that submodules are included
    for _, name in ipairs({
        'type',
        'message',
        'check',
    }) do
        assert.is_table(error[name])
    end
end

function testcase.call()
    -- test that behavior of __call metamethod is the same as built-in error func
    for _, lv in ipairs({
        -1,
        0,
        1,
        2,
    }) do
        local errs = {}
        for _, f in ipairs({
            error,
            builtin_error,
        }) do
            local err = assert.throws(function()
                f('foo', lv)
            end)
            errs[#errs + 1] = err
        end
        assert.equal(errs[1], errs[2])
    end
end

function testcase.new()
    -- test that create new error with string
    local err = error.new('hello error')
    assert.match(tostring(err), 'error_test%.lua.+ hello error', false)

    -- test that create new error with level
    local fncall = function(fn)
        fn()
    end
    fncall(function()
        err = error.new('hello error', nil, 2)
        assert.match(tostring(err), 'in .+fncall', false)
    end)

    -- test that create new error with level and trace
    err = error.new('hello error', nil, nil, true)
    assert.match(tostring(err), 'stack traceback:.+', false)

    -- test that create new error with table that contains __tostring metamethod
    err = error.new(setmetatable({
        err = 'hello error',
    }, {
        __tostring = function(self)
            return self.err .. ' from __tostring'
        end,
    }))
    assert.match(tostring(err), 'error_test%.lua.+ hello error from __tostring',
                 false)

    -- test that throw error
    for _, v in ipairs({
        -- invalid wrap argument
        {
            arg = {
                'hello',
                true,
            },
            match = '#2 .+error expected',
        },
        -- invalid level argument
        {
            arg = {
                'hello',
                nil,
                'foo',
            },
            match = '#3 .+positive integer expected',
        },
        -- invalid traceback argument
        {
            arg = {
                'hello',
                nil,
                nil,
                'foo',
            },
            match = '#4 .+boolean expected',
        },
    }) do
        err = assert.throws(function()
            error.new(unpack(v.arg))
        end)
        assert.match(err, v.match, false)
    end
end

function testcase.properties()
    -- test that accessing properties
    local err = error.new('hello error')
    local msg = assert(err.message)
    assert.match(tostring(msg), 'hello error')
    assert.is_nil(err.type)
    assert.is_nil(err.op)
    assert.equal(err.code, -1)
end

function testcase.tostring()
    -- test that string conversion
    local err = error.new('hello error')
    assert.match(err, 'error_test%.lua.+ hello error', false)

    -- test that convert with wrapped error
    local err2 = error.new('world error', err)
    assert.match(err2, 'error_test%.lua.+ hello error', false)
    assert.match(err2, 'error_test%.lua.+ world error', false)

    -- test that throws an error if __tostring metamethod not returns a string
    err = error.new(setmetatable({}, {
        __tostring = function()
            return true
        end,
    }))
    err = assert.throws(tostring, err)
    assert.match(err, 'must return a string')
end

function testcase.cause()
    for _, v in ipairs({
        'hello error',
        {
            err = 'hello error',
            tostring = function(self, where)
                return where .. self.err .. ' from tostring'
            end,
        },
        setmetatable({
            err = 'hello error',
        }, {
            __tostring = function(self)
                return self.err .. ' from __tostring'
            end,
        }),
    }) do
        -- test that return the data contained in the error
        local err = error.new(v)
        local msg, is_msg = error.cause(err)
        assert.is_true(is_msg)
        assert.equal(msg.message, v)
    end

    -- test that return first argument
    local msg = error.message.new('error message', 'test message')
    local cmsg, is_msg = error.cause(msg)
    assert.is_true(is_msg)
    assert.equal(cmsg, msg)

    cmsg, is_msg = error.cause('foo')
    assert.is_false(is_msg)
    assert.equal(cmsg, 'foo')

    cmsg, is_msg = error.cause({
        'foo',
    })
    assert.is_false(is_msg)
    assert.equal(cmsg, {
        'foo',
    })

    -- test that returns nil if no argument
    assert.is_nil(error.cause())
end

function testcase.unwrap()
    local err
    for _, v in ipairs({
        'hello error',
        setmetatable({
            err = 'hello error',
        }, {
            __tostring = function(self)
                return self.err .. ' from __tostring'
            end,
        }),
    }) do
        -- test that create new error that wraps another error
        local newerr = error.new(v, err)
        assert.equal(error.cause(newerr).message, v)

        -- test that return wrapped error
        assert.equal(error.unwrap(newerr), err)
        err = newerr
    end
end

function testcase.toerror()
    -- test that create new error with string
    local err = error.toerror('hello error')
    assert.match(tostring(err), 'error_test%.lua.+ hello error', false)

    -- test that return passed error
    assert.rawequal(error.toerror(err, 'foo'), err)
end

function testcase.debug()
    -- test that the traceback argument is forced to be true if sets the debug flag is true
    error.debug(true)
    local err = error.new('hello error', nil, nil, false)
    assert.match(tostring(err), 'stack traceback')
    error.debug(false)
    err = error.new('hello error', nil, nil, false)
    assert.not_match(tostring(err), 'stack traceback')
end

testcase()
