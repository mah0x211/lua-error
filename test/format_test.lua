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

    -- 整数型: d, i, o, u, x, X
    v = error.format('%+d %-5i %05o %u %#x %#X %ld %d %d', 42, 42, 42, 42, 42, 42, 42, true, false)
    assert.match(v, "+42 42    00052 42 0x2a 0X2A 42 1 0")

    -- 浮動小数点数型: e, E, f, F, g, G
    v = error.format("%+e %-.*E %+f % F %.1g %.1G", 1.23, 2, 1.23, 1.23, 1.23,
                     1.23, 1.23)
    assert.match(v, "+1.230000e+00 1.23E+00 +1.230000  1.230000 1 1")

    -- 浮動小数点数型 16進数: a, A
    v = error.format("%a %#A", 1.23, 1.23)
    assert.match(v, "0x1%.[a-f0-9p+]+ 0X1%.[A-F0-9P+]+", false)

    -- 文字と文字列: c, s
    v = error.format("%-3c %*s %s %s", 'A', 6, "hello", true, false)
    assert.match(v, "A    hello true false")

    -- ポインタ: p
    v = error.format("%p", {})
    assert.match(v, "0x[0-9a-f]+", false)

    -- % のエスケープ
    v = error.format("%%")
    assert.match(v, "%")

    -- errnoを設定して%mのテスト
    v = error.format("%m")
    assert(v ~= nil)
end

