require('luacov')
local _, tester = assert(pcall(function()
    package.path = './?.lua;./test/?.lua;' .. package.path
    return require('./tester')
end))
local testcase = tester()
local assert = require('assert')
local fatalf = require('error.fatalf')

function testcase.fatalf()
    -- test that error.fatalf() throws an error
    local err = assert.throws(fatalf, 'hello world')
    assert.match(err, 'hello world')

    -- test that arguments are concatenated into a message string with spaces
    err = assert.throws(fatalf, 'hello', 'error format')
    assert.match(err, 'hello error format')

    -- test that error message is formatted with arguments
    err = assert.throws(fatalf, 'hello %p', 'error', {})
    assert.re_match(err, 'hello (\\(nil\\)|0x[0-9a-f]+) {}')

    -- test that level option can be specified at the first argument
    local ok
    ok, err = pcall(function()
        local function nestfn()
            fatalf(2, 'hello %p', 'error', '{}')
        end
        nestfn()
    end)
    assert.is_false(ok)
    assert.re_match(err, 'fatalf_test\\.lua:29')
end

testcase()
