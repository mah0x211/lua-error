local unpack = unpack or table.unpack
local assert = require('assertex')
local testcase = require('testcase')
local error = require('error')
local error_type = error.type

function testcase.after_each()
    error_type.del('my.error')
end

function testcase.after_all()
    error_type.del('my.error')
end

function testcase.new()
    -- test that create new error type
    local t = error_type.new('my.error')
    assert.match(tostring(t), '^my.error: ', false)

    -- test that throw error
    for _, v in ipairs({
        -- invalid value type
        {
            arg = 123,
            match = '#1 .+string expected',
        },
        -- invalid first letter
        {
            arg = ' my.error',
            match = '#1 .+first letter to be alphabetic character',
        },
        -- contains invalid character
        {
            arg = 'my error',
            match = [[#1 .+alphanumeric or '_' or '%.' characters expected]],
        },
        -- argument length too long
        {
            arg = '',
            match = '#1 .+string length between 1%-127 expected, got 0',
        },
        {
            arg = string.rep('f', 128),
            match = '#1 .+string length between 1%-127 expected, got 128',
        },
    }) do
        local err = assert.throws(function()
            error_type.new(v.arg)
        end)
        assert.match(err, v.match, false)
    end
end

function testcase.get()
    do
        local t = error_type.new('my.error')

        -- test that get registered error type
        collectgarbage('collect')
        assert.rawequal(error_type.get('my.error'), t)
    end

    -- verify that the error type objects are helds in a weak reference table
    collectgarbage('collect')
    assert.is_nil(error_type.get('my.error'))
end

function testcase.del()
    local _ = error_type.new('my.error')
    -- test that delete registered error type
    assert.is_true(error_type.del('my.error'))
    assert.is_false(error_type.del('my.error'))
end

function testcase.name()
    -- test that return type name
    local t = error_type.new('my.error')
    assert.equal(t:name(), 'my.error')
end

function testcase.new_error()
    -- test that create new typed error
    local t = error_type.new('my.error')
    local err = t:new({
        tostring = function(_, where, traceback)
            assert.is_nil(traceback)
            return where .. 'typed error'
        end,
    })
    assert.match(tostring(err), 'error_type_test%.lua.+ typed error', false)

    -- test that create new typed error with a stack traceback
    err = t:new({
        tostring = function(_, where, traceback)
            assert.is_string(traceback)
            return where .. 'error with traceback\n' .. traceback
        end,
    }, nil, nil, true)
    assert.match(tostring(err), 'stack traceback:', false)

    -- test that throw error
    for _, v in ipairs({
        -- no message
        {
            arg = {},
            match = '#1 .+argument must be string or table expected, got no value',
        },
        -- invalid message
        {
            arg = {
                true,
            },
            match = '#1 .+argument must be string or table expected, got boolean',
        },
        {
            arg = {
                1,
            },
            match = '#1 .+argument must be string or table expected, got number',
        },
        {
            arg = {
                {},
            },
            match = '#1 .+tostring function or __tostring metamethod does not exist in table',
        },
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
            match = '#3 .+number expected',
        },
        {
            arg = {
                'hello',
                nil,
                -1,
            },
            match = '#3 .+uint8_t expected',
        },
        {
            arg = {
                'hello',
                nil,
                256,
            },
            match = '#3 .+uint8_t expected',
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
            t:new(unpack(v.arg))
        end)
        assert.match(err, v.match, false)
    end
end

function testcase.is()
    local t = error_type.new('my.error')
    local str = 'hello error'
    local tbl = {
        err = 'hello error',
        tostring = function(self, where)
            return where .. self.err .. ' from tostring'
        end,
    }
    local obj = setmetatable({
        err = 'hello error',
    }, {
        __tostring = function(self, where)
            return (where or '') .. self.err .. ' from __tostring'
        end,
    })
    local err_str = error.new(str)
    local err_typed_err = t:new(tbl, err_str)
    local err_obj = error.new(obj, err_typed_err)
    local err = error.new('wrap errors', err_obj)

    -- test that return err_typed_err
    assert.rawequal(error.is(err, t), err_typed_err)
end

function testcase.typeof()
    local t = error_type.new('my.error')
    local err = t:new('hello typed error')

    -- test that return error type object associated with err
    assert.rawequal(error.typeof(err), t)
end

