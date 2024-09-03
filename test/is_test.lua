require('luacov')
local _, tester = assert(pcall(function()
    package.path = './?.lua;./test/?.lua;' .. package.path
    return require('./tester')
end))
local testcase = tester()
local assert = require('assert')
local error = require('error')
local error_type = error.type

function testcase.after_each()
    error_type.del('my.error')
end

function testcase.after_all()
    error_type.del('my.error')
end

function testcase.is()
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
    local t = assert(error_type.new('my.error'))
    local typed_err = t:new('hello typed error')
    local err_str = error.new(str, typed_err)
    local err_tbl = error.new(tbl, err_str)
    local err_obj = error.new(obj, err_tbl)
    local err = error.new('wrap errors', err_obj)

    -- test that return err_str
    assert.rawequal(error.is(err, str), err_str)
    assert.rawequal(error.is(err, err_str), err_str)

    -- test that return err_tbl
    assert.rawequal(error.is(err, tbl), err_tbl)
    assert.rawequal(error.is(err, err_tbl), err_tbl)

    -- test that return err_obj
    assert.rawequal(error.is(err, obj), err_obj)
    assert.rawequal(error.is(err, err_obj), err_obj)

    -- test that return typed_err
    assert.rawequal(error.is(err, t), typed_err)
    assert.rawequal(error.is(err, typed_err), typed_err)

    -- test that return typed_err by type name
    assert.rawequal(error.is(err, 'my.error'), typed_err)

    -- test that return a nil if no match
    assert.is_nil(error.is(err, 'foo'))

    -- test that return nil if no arguments
    assert.is_nil(error.is())

    -- test that return nil if first argument is not an error object
    assert.is_nil(error.is('foo'))

    -- test that return nil if no second argument
    assert.is_nil(error.is(err))
end

testcase()
