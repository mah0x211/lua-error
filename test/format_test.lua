local assert = require('assert')
local testcase = require('testcase')
local error = require('error')

function testcase.format()
    -- test that error.format() returns a string
    local hello_error = error.format('hello world')
    assert.match(hello_error, 'hello world')

    -- test that format with error object
    local v = error.format('hello %p', 'error', hello_error)
    assert.re_match(v, 'hello (\\(nil\\)|0x[0-9a-f]+)')
    assert.equal(error.unwrap(v), hello_error)

    -- test that the error object argument is concatenated into a message
    -- string if it is not an error object
    v = error.format('hello', 'error format')
    assert.match(v, 'hello: error format')
    assert.is_nil(error.unwrap(v))

    -- test that integer type: d, i, o, u, x, X
    v = error.format('%+d %-5i %05o %u %#x %#X %ld %d %d', 42, 42, 42, 42, 42,
                     42, 42, true, false)
    assert.match(v, "+42 42    00052 42 0x2a 0X2A 42 1 0")

    -- test that floting point type: e, E, f, F, g, G
    v = error.format("%+e %-.*E %+f % F %.1g %.1G", 1.23, 2, 1.23, 1.23, 1.23,
                     1.23, 1.23)
    assert.match(v, "+1.230000e+00 1.23E+00 +1.230000  1.230000 1 1")

    -- test that floating point in hexdigit: a, A
    v = error.format("%a %#A", 1.23, 1.23)
    assert.match(v, "0x1%.[a-f0-9p+]+ 0X1%.[A-F0-9P+]+", false)

    -- test that character and string type: c, s
    v = error.format("%-3c %*s %s %s", 'A', 6, "hello", true, false)
    assert.match(v, "A    hello true false")

    -- test that pointer type: p
    v = error.format("%p", {})
    assert.match(v, "0x[0-9a-f]+", false)

    -- test that escape: %
    v = error.format("%%")
    assert.match(v, "%")

    -- test that print errno: m
    v = error.format("%m")
    assert(v ~= nil)

    -- test that quoted string: q
    v = error.format("%q", 'a\t' .. string.char(0) .. '1\bx\ad\f\n"\rあ\v𠀋')
    assert.match(v, '\"a\\t\\01\\bx\\ad\\f\\n\\"\\rあ\\v𠀋"')
end

