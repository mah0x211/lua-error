local testcase = require('testcase')
local error_check = require('error').check

function testcase.check()
    for fn, v in pairs({
        -- file
        [error_check.file] = {
            valid = {
                io.open('/dev/null'),
            },
            invalid = {
                {
                    arg = 'foo',
                    match = 'FILE%* expected, got string',
                },
            },
        },
        -- boolean
        [error_check.boolean] = {
            valid = {
                true,
                false,
            },
            invalid = {
                {
                    arg = 'foo',
                    match = 'boolean expected, got string',
                },
            },
        },
        -- string
        [error_check.string] = {
            valid = {
                'foo',
            },
            invalid = {
                {
                    arg = 1,
                    match = 'string expected, got number',
                },
            },
        },
        -- table
        [error_check.table] = {
            valid = {
                {},
            },
            invalid = {
                {
                    arg = 1,
                    match = 'table expected, got number',
                },
            },
        },
        -- func
        [error_check.func] = {
            valid = {
                function()
                end,
            },
            invalid = {
                {
                    arg = 1,
                    match = 'function expected, got number',
                },
            },
        },
        -- userdata
        [error_check.userdata] = {
            valid = {
                assert(io.open('/dev/null')),
            },
            invalid = {
                {
                    arg = require('assert.lightuserdata'),
                    match = 'userdata expected, got pointer',
                },
            },
        },
        -- pointer
        [error_check.pointer] = {
            valid = {
                require('assert.lightuserdata'),
                nil,
            },
            invalid = {
                {
                    arg = 1,
                    match = 'pointer expected, got number',
                },
            },
        },
        -- thread
        [error_check.thread] = {
            valid = {
                coroutine.create(function()
                end),
            },
            invalid = {
                {
                    arg = 1,
                    match = 'thread expected, got number',
                },
            },
        },
        -- number
        [error_check.number] = {
            valid = {
                -1.4,
                -1,
                1,
                1.2,
                math.huge,
                -math.huge,
            },
            invalid = {
                {
                    arg = 'foo',
                    match = 'number expected, got string',
                },
                {
                    arg = 0 / 0,
                    match = 'number expected, got %-*nan',
                },
            },
        },
        -- unsigned
        [error_check.unsigned] = {
            valid = {
                1,
                1.234,
                math.huge,
            },
            invalid = {
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
            },
        },
        -- finite
        [error_check.finite] = {
            valid = {
                -1,
                -1.234,
                1,
                1.234,
            },
            invalid = {
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
            },
        },
        -- int
        [error_check.int] = {
            valid = {
                -1,
                0,
                1,
                1234,
            },
            invalid = {
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
            },
        },
        -- int8
        [error_check.int8] = {
            valid = {
                -128,
                127,
            },
            invalid = {
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
            },
        },
        -- int16
        [error_check.int16] = {
            valid = {
                -32768,
                32767,
            },
            invalid = {
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
            },
        },
        -- int32
        [error_check.int32] = {
            valid = {
                -2147483648,
                2147483647,
            },
            invalid = {
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
            },
        },
        -- uint
        [error_check.uint] = {
            valid = {
                0,
                1,
                1234,
            },
            invalid = {
                {
                    arg = 'foo',
                    match = 'uint expected, got string',
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
            },
        },
        -- uint8
        [error_check.uint8] = {
            valid = {
                0,
                255,
            },
            invalid = {
                {
                    arg = 'string',
                    match = 'uint8 expected, got string',
                },
                {
                    arg = 256,
                    match = 'uint8 expected, got an out of range value',
                },
            },
        },
        -- uint16
        [error_check.uint16] = {
            valid = {
                0,
                65535,
            },
            invalid = {
                {
                    arg = 'string',
                    match = 'uint16 expected, got string',
                },
                {
                    arg = 65536,
                    match = 'uint16 expected, got an out of range value',
                },
            },
        },
        -- uint32
        [error_check.uint32] = {
            valid = {
                0,
                4294967295,
            },
            invalid = {
                {
                    arg = 'string',
                    match = 'uint32 expected, got string',
                },
                {
                    arg = 4294967296,
                    match = 'uint32 expected, got an out of range value',
                },
            },
        },
    }) do
        -- test that valid argument
        for _, arg in ipairs(v.valid) do
            fn(arg)
        end

        for _, invalid in ipairs(v.invalid) do
            local function testcall(arg, idx, level, traceback)
                fn(arg, idx, level, traceback)
            end

            local function invoke(...)
                testcall(...)
            end

            -- test that invalid argument
            local err = assert.throws(function()
                invoke(invalid.arg)
            end)
            assert.match(err, invalid.match, false)
            assert.match(err, '#1 .+testcall', false)
            assert.match(err, 'stack traceback:', false)

            -- test that invalid argument with argidx
            err = assert.throws(function()
                invoke(invalid.arg, 5)
            end)
            assert.match(err, '#5 .+testcall', false)

            -- test that invalid argument with level and without traceback
            err = assert.throws(function()
                invoke(invalid.arg, 4, 2, false)
            end)
            assert.match(err, '#4 .+invoke', false)
            assert.not_match(err, 'stack traceback:', false)
        end
    end
end

