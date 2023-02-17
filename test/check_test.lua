local testcase = require('testcase')
local error_check = require('error').check

local function testcall(checkfn, arg, idx, level, traceback)
    checkfn(arg, idx, level, traceback)
end

local function invoke(...)
    testcall(...)
end

function testcase.noneornil()

    -- test that valid argument
    error_check.noneornil()
    error_check.noneornil(nil)

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.noneornil, 'foo')
    end)
    assert.match(err, 'none or nil expected, got string', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.noneornil, 'foo', 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.noneornil, 'foo', 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.file()
    -- test that valid argument
    error_check.file(io.open('/dev/null'))

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.file, 'foo')
    end)
    assert.match(err, 'FILE%* expected, got string', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.file, 'foo', 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.file, 'foo', 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.boolean()
    -- test that valid argument
    for _, arg in ipairs({
        true,
        false,
    }) do
        error_check.boolean(arg)
    end

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.boolean, 'foo')
    end)
    assert.match(err, 'boolean expected, got string', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.boolean, 'foo', 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.boolean, 'foo', 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.string()
    -- test that valid argument
    error_check.string('foo')

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.string, 1)
    end)
    assert.match(err, 'string expected, got number', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.string, 1, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.string, 1, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.table()
    -- test that valid argument
    error_check.table({})

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.table, 1)
    end)
    assert.match(err, 'table expected, got number', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.table, 1, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.table, 1, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.func()
    -- test that valid argument
    error_check.func(function()
    end)

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.func, 1)
    end)
    assert.match(err, 'function expected, got number', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.func, 1, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.func, 1, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.userdata()
    -- test that valid argument
    error_check.userdata(assert(io.open('/dev/null')))

    -- test that invalid argument
    local arg = require('assert.lightuserdata')
    local err = assert.throws(function()
        invoke(error_check.userdata, arg)
    end)
    assert.match(err, 'userdata expected, got pointer', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.userdata, arg, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.userdata, arg, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.pointer()
    -- test that valid argument
    error_check.pointer(require('assert.lightuserdata'))

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.pointer, 1)
    end)
    assert.match(err, 'pointer expected, got number', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.pointer, 1, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.pointer, 1, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.thread()
    -- test that valid argument
    error_check.thread(coroutine.create(function()
    end))

    -- test that invalid argument
    local err = assert.throws(function()
        invoke(error_check.thread, 1)
    end)
    assert.match(err, 'thread expected, got number', false)
    assert.match(err, '#1 .+testcall', false)
    assert.match(err, 'stack traceback:', false)

    -- test that invalid argument with argidx
    err = assert.throws(function()
        invoke(error_check.thread, 1, 5)
    end)
    assert.match(err, '#5 .+testcall', false)

    -- test that invalid argument with level and without traceback
    err = assert.throws(function()
        invoke(error_check.thread, 1, 4, 2, false)
    end)
    assert.match(err, '#4 .+invoke', false)
    assert.not_match(err, 'stack traceback:', false)
end

function testcase.number()
    -- test that valid argument
    for _, arg in ipairs({
        -1.4,
        -1,
        1,
        1.2,
        math.huge,
        -math.huge,
    }) do
        error_check.number(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'number expected, got string',
        },
        {
            arg = 0 / 0,
            match = 'number expected, got %-*nan',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.number, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.number, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.number, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.unsigned()
    -- test that valid argument
    for _, arg in ipairs({
        1,
        1.234,
        math.huge,
    }) do
        error_check.unsigned(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'unsigned number expected, got string',
        },
        {
            arg = -1.2,
            match = 'unsigned number expected, got signed number',
        },
        {
            arg = 0 / 0,
            match = 'unsigned number expected, got %-*nan',
        },
    }) do

        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.unsigned, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.unsigned, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.unsigned, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.finite()
    -- test that valid argument
    for _, arg in ipairs({
        -1,
        -1.234,
        1,
        1.234,
    }) do
        error_check.finite(arg)
    end

    for _, invalid in ipairs({
        {
            arg = math.huge,
            match = 'finite number expected, got inf',
        },
        {
            arg = -math.huge,
            match = 'finite number expected, got %-inf',
        },
        {
            arg = 0 / 0,
            match = 'finite number expected, got %-*nan',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.finite, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.finite, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.finite, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.int()
    -- test that valid argument
    for _, arg in ipairs({
        -1,
        0,
        1,
        1234,
    }) do
        error_check.int(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'int expected, got string',
        },
        {
            arg = 1.2,
            match = 'int expected, got number',
        },
        {
            arg = 0 / 0,
            match = 'int expected, got %-*nan',
        },
        {
            arg = math.huge,
            match = 'int expected, got inf',
        },
        {
            arg = -math.huge,
            match = 'int expected, got %-inf',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.int, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.int, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.int, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.int8()
    -- test that valid argument
    for _, arg in ipairs({
        -128,
        127,
    }) do
        error_check.int8(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'int8 expected, got string',
        },
        {
            arg = -129,
            match = 'int8 expected, got an out of range value',
        },
        {
            arg = 128,
            match = 'int8 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.int8, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.int8, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.int8, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.int16()
    -- test that valid argument
    for _, arg in ipairs({
        -32768,
        32767,
    }) do
        error_check.int16(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'int16 expected, got string',
        },
        {
            arg = -32769,
            match = 'int16 expected, got an out of range value',
        },
        {
            arg = 32768,
            match = 'int16 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.int16, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.int16, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.int16, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.int32()
    -- test that valid argument
    for _, arg in ipairs({
        -2147483648,
        2147483647,
    }) do
        error_check.int32(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'int32 expected, got string',
        },
        {
            arg = -2147483649,
            match = 'int32 expected, got an out of range value',
        },
        {
            arg = 2147483648,
            match = 'int32 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.int32, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.int32, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.int32, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.uint()
    -- test that valid argument
    for _, arg in ipairs({
        0,
        1,
        1234,
    }) do
        error_check.uint(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'uint expected, got string',
        },
        {
            arg = -1,
            match = 'uint expected, got an out of range value',
        },
        {
            arg = 1.2,
            match = 'uint expected, got number',
        },
        {
            arg = 0 / 0,
            match = 'uint expected, got %-*nan',
        },
        {
            arg = math.huge,
            match = 'uint expected, got inf',
        },
        {
            arg = -math.huge,
            match = 'uint expected, got %-inf',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.uint, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.uint, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.uint, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.uint8()
    -- test that valid argument
    for _, arg in ipairs({
        0,
        255,
    }) do
        error_check.uint8(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'string',
            match = 'uint8 expected, got string',
        },
        {
            arg = -1,
            match = 'uint8 expected, got an out of range value',
        },
        {
            arg = 256,
            match = 'uint8 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.uint8, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.uint8, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.uint8, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.uint16()
    -- test that valid argument
    for _, arg in ipairs({
        0,
        65535,
    }) do
        error_check.uint16(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'string',
            match = 'uint16 expected, got string',
        },
        {
            arg = -1,
            match = 'uint16 expected, got an out of range value',
        },
        {
            arg = 65536,
            match = 'uint16 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.uint16, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.uint16, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.uint16, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.uint32()
    -- test that valid argument
    for _, arg in ipairs({
        0,
        4294967295,
    }) do
        error_check.uint32(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'string',
            match = 'uint32 expected, got string',
        },
        {
            arg = -1,
            match = 'uint32 expected, got an out of range value',
        },
        {
            arg = 4294967296,
            match = 'uint32 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.uint32, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.uint32, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.uint32, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.pint()
    -- test that valid argument
    for _, arg in ipairs({
        1,
        1234,
    }) do
        error_check.pint(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'foo',
            match = 'pint expected, got string',
        },
        {
            arg = 0,
            match = 'pint expected, got an out of range value',
        },
        {
            arg = 1.2,
            match = 'pint expected, got number',
        },
        {
            arg = 0 / 0,
            match = 'pint expected, got %-*nan',
        },
        {
            arg = math.huge,
            match = 'pint expected, got inf',
        },
        {
            arg = -math.huge,
            match = 'pint expected, got %-inf',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.pint, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.pint, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.pint, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.pint8()
    -- test that valid argument
    for _, arg in ipairs({
        1,
        255,
    }) do
        error_check.pint8(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'string',
            match = 'pint8 expected, got string',
        },
        {
            arg = 0,
            match = 'pint8 expected, got an out of range value',
        },
        {
            arg = 256,
            match = 'pint8 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.pint8, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.pint8, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.pint8, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

function testcase.pint16()
    -- test that valid argument
    for _, arg in ipairs({
        1,
        65535,
    }) do
        error_check.pint16(arg)
    end

    for _, invalid in ipairs({
        {
            arg = 'string',
            match = 'pint16 expected, got string',
        },
        {
            arg = 0,
            match = 'pint16 expected, got an out of range value',
        },
        {
            arg = 65536,
            match = 'pint16 expected, got an out of range value',
        },
    }) do
        -- test that invalid argument
        local err = assert.throws(function()
            invoke(error_check.pint16, invalid.arg)
        end)
        assert.match(err, invalid.match, false)
        assert.match(err, '#1 .+testcall', false)
        assert.match(err, 'stack traceback:', false)

        -- test that invalid argument with argidx
        err = assert.throws(function()
            invoke(error_check.pint16, invalid.arg, 5)
        end)
        assert.match(err, '#5 .+testcall', false)

        -- test that invalid argument with level and without traceback
        err = assert.throws(function()
            invoke(error_check.pint16, invalid.arg, 4, 2, false)
        end)
        assert.match(err, '#4 .+invoke', false)
        assert.not_match(err, 'stack traceback:', false)
    end
end

