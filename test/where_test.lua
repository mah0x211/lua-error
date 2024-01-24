require('luacov')
local _, tester = assert(pcall(function()
    package.path = './?.lua;./test/?.lua;' .. package.path
    return require('./tester')
end))
local testcase = tester()
local assert = require('assert')
local where = require('error.where')

function testcase.where()
    -- test that where returns a location of the caller
    local err = where()
    assert.match(err, 'where_test.lua:12')

    -- test that where returns a location of the caller of testcase.where
    for level, cmp in ipairs({
        'where_test\\.lua:26',
        'in .+xpcall',
        'tester\\.lua',
        'tester\\.lua',
        'in main chunk',
        '\\?:',
        '', -- out of range
        '',
    }) do
        err = where(level)
        if cmp == '' then
            assert.equal(err, cmp)
        else
            assert.re_match(err, cmp)
        end
    end

    -- -- test that arguments are concatenated into a message string with spaces
    -- err = assert.throws(fatalf, 'hello', 'error format')
    -- assert.match(err, 'hello error format')

    -- -- test that error message is formatted with arguments
    -- err = assert.throws(fatalf, 'hello %p', 'error', {})
    -- assert.re_match(err, 'hello (\\(nil\\)|0x[0-9a-f]+) table: 0x[0-9a-f]+')

    -- -- test that level option can be specified at the first argument
    -- local ok
    -- ok, err = pcall(function()
    --     local function nestfn()
    --         fatalf(2, 'hello %p', 'error', {})
    --     end
    --     nestfn()
    -- end)
    -- assert.is_false(ok)
    -- assert.re_match(err, 'fatalf_test\\.lua:29')
end

testcase()
