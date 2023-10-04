local assert = require('assert')
local testcase = require('testcase')
local error = require('error')

function testcase.format()
    -- test that error.fatalf() throws an error
    local err = assert.throws(error.fatalf, 'hello world')
    assert.match(err, 'hello world')

    -- test that arguments are concatenated into a message string with spaces
    err = assert.throws(error.fatalf, 'hello', 'error format')
    assert.match(err, 'hello error format')

    -- test that error message is formatted with arguments
    err = assert.throws(error.fatalf, 'hello %p', 'error', {})
    assert.re_match(err, 'hello (\\(nil\\)|0x[0-9a-f]+) table: 0x[0-9a-f]+')

    -- test that level option can be specified at the first argument
    local ok
    ok, err = pcall(function()
        local function nestfn()
            error.fatalf(2, 'hello %p', 'error', {})
        end
        nestfn()
    end)
    assert.is_false(ok)
    assert.re_match(err, 'fatalf_test\\.lua:24')
end

