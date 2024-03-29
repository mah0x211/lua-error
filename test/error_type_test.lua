require('luacov')
local _, tester = assert(pcall(function()
    package.path = './?.lua;./test/?.lua;' .. package.path
    return require('./tester')
end))
local testcase = tester()
local assert = require('assert')
local unpack = require('unpack')
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

    -- test that create new error type with code
    t = error_type.new('my.error2', 123)
    assert.match(tostring(t), '^my.error2: ', false)

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
            match = '#1 .+format is "^%%a%[%%w_%.]%*$" expected',
        },
        -- contains invalid character
        {
            arg = 'my error',
            match = '#1 .+format is "^%%a%[%%w_%.]%*$" expected',
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

function testcase.gced()
    -- test that an error-types are kept in the registry as a weak references
    local err
    do
        -- luacheck: ignore t1
        local t1 = error_type.new('my.error1')
        local t2 = error_type.new('my.error2')
        err = t2:new('message')
    end
    collectgarbage('collect')
    assert(err ~= nil)
    assert.is_nil(error_type.get('my.error'))
    local t2 = error_type.get('my.error2')
    assert.equal(t2.name, 'my.error2')
end

function testcase.name()
    -- test that return type name
    local t = error_type.new('my.error')
    assert.equal(t.name, 'my.error')
end

function testcase.code()
    -- test that return type code
    local t = error_type.new('my.error')
    assert.equal(t.code, -1)

    t = error_type.new('my.error2', 123)
    assert.equal(t.code, 123)

    local err = t:new()
    assert.equal(err.code, 123)
end

function testcase.message()
    -- test that return type message
    local t = error_type.new('my.error1')
    assert.is_nil(t.message)

    -- test that add type message
    t = error_type.new('my.error', nil, 'hello world!')
    assert.equal(t.message, 'hello world!')
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

function testcase.new_error()
    -- test that create new typed error without message
    local t = error_type.new('my.error1', nil, 'main error message')
    local err = t:new()
    assert.match(err,
                 'error_type_test%.lua.+ %[my.error1:%-1] main error message$',
                 false)

    -- test that create new typed error with string message
    err = t:new('typed string error')
    assert.match(tostring(err),
                 'error_type_test%.lua.+ %[my.error1:%-1].+ [(]typed string error[)]',
                 false)

    -- test that create new typed error with structured message
    t = error_type.new('my.error')
    err = t:new(setmetatable({}, {
        __tostring = function()
            return 'typed error'
        end,
    }))
    assert.match(tostring(err),
                 'error_type_test%.lua.+ %[my.error:%-1] typed error', false)

    -- test that create new typed error with a stack traceback
    err = t:new('error with traceback', nil, nil, true)
    assert.match(tostring(err), 'stack traceback:', false)

    -- test that create new typed error without message
    err = t:new()
    assert.match(tostring(err), '%[my.error:%-1] %(no message%)', false)

    -- test that throw error if name already exists
    err = assert.throws(error_type.new, 'my.error')
    assert.match(err, 'already used in other error type')

    -- test that throw error if argument is invalid
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
            match = '#3 .+integer expected',
        },
        {
            arg = {
                'hello',
                nil,
                -1,
            },
            match = '#3 .+positive integer expected',
        },
        {
            arg = {
                'hello',
                nil,
                0,
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
        tostring = function(self)
            return self.err .. ' from tostring'
        end,
    }
    local obj = setmetatable({
        err = 'hello error',
    }, {
        __tostring = function(self)
            return self.err .. ' from __tostring'
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

    -- test that return error.type object associated with err
    assert.rawequal(error.typeof(err), t)

    -- test that return error.type object
    assert.rawequal(error.typeof(t), t)

    -- test that return nil
    assert.is_nil(error.typeof('foo'))
    assert.is_nil(error.typeof())
end

testcase()
