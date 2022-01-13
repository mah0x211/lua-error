local unpack = unpack or table.unpack
local builtin_error = error
local testcase = require('testcase')
local error = require('error')

function testcase.after_each()
    error.debug(false)
end

function testcase.after_all()
    error.debug(false)
end

function testcase.check_submodules()
    -- test that submodules are included
    for _, name in ipairs({
        'type',
        'message',
        'check',
        'errno',
    }) do
        assert.is_table(error[name])
    end
end

function testcase.call()
    -- test that behavior of __call metamethod is the same as built-in error func
    for _, lv in ipairs({
        -1,
        0,
        1,
        2,
    }) do
        local errs = {}
        for _, f in ipairs({
            error,
            builtin_error,
        }) do
            local err = assert.throws(function()
                f('foo', lv)
            end)
            errs[#errs + 1] = err
        end
        assert.equal(errs[1], errs[2])
    end
end

function testcase.new()
    -- test that create new error with string
    local err = error.new('hello error')
    assert.match(tostring(err), 'error_test%.lua.+ hello error', false)

    -- test that create new error with level
    local fncall = function(fn)
        fn()
    end
    fncall(function()
        err = error.new('hello error', nil, 2)
        assert.match(tostring(err), 'in .+fncall', false)
    end)

    -- test that create new error with level and trace
    err = error.new('hello error', nil, nil, true)
    assert.match(tostring(err), 'stack traceback:.+', false)

    -- test that create new error with table that contains tostring function
    err = error.new({
        err = 'hello error',
        tostring = function(self, where)
            return where .. self.err .. ' from tostring'
        end,
    })
    assert.match(tostring(err), 'error_test%.lua.+ hello error from tostring',
                 false)

    -- test that create new error with table that contains __tostring metamethod
    err = error.new(setmetatable({
        err = 'hello error',
    }, {
        __tostring = function(self, where)
            return where .. self.err .. ' from __tostring'
        end,
    }))
    assert.match(tostring(err), 'error_test%.lua.+ hello error from __tostring',
                 false)

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
            error.new(unpack(v.arg))
        end)
        assert.match(err, v.match, false)
    end
end

function testcase.cause()
    for _, v in ipairs({
        'hello error',
        {
            err = 'hello error',
            tostring = function(self, where)
                return where .. self.err .. ' from tostring'
            end,
        },
        setmetatable({
            err = 'hello error',
        }, {
            __tostring = function(self, where)
                return (where or '') .. self.err .. ' from __tostring'
            end,
        }),
    }) do
        -- test that return the data contained in the error
        local err = error.new(v)
        assert.rawequal(error.cause(err), v)
    end
end

function testcase.unwrap()
    local err
    for _, v in ipairs({
        'hello error',
        {
            err = 'hello error',
            tostring = function(self, where)
                return where .. self.err .. ' from tostring'
            end,
        },
        setmetatable({
            err = 'hello error',
        }, {
            __tostring = function(self, where)
                return (where or '') .. self.err .. ' from __tostring'
            end,
        }),
    }) do
        -- test that create new error that wraps another error
        local newerr = error.new(v, err)
        assert.equal(error.cause(newerr), v)

        -- test that return wrapped error
        assert.equal(error.unwrap(newerr), err)
        err = newerr
    end
end

function testcase.is()
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

    -- test that return nil if no arguments
    assert.is_nil(error.is())

    -- test that return nil if first argument is not an error object
    assert.is_nil(error.is('foo'))

    -- test that return nil if no second argument
    assert.is_nil(error.is(err))

    -- test that throws an error if second argument is invalid
    err = assert.throws(function()
        error.is(err, 12.34)
    end)
    assert.match(err, '#2 .+string, table, error or error_type expected', false)
end

function testcase.toerror()
    -- test that create new error with string
    local err = error.toerror('hello error')
    assert.match(tostring(err), 'error_test%.lua.+ hello error', false)

    -- test that return passed error
    assert.rawequal(error.toerror(err, 'foo'), err)
end

function testcase.debug()
    -- test that the traceback argument is forced to be true if sets the debug flag is true
    error.debug(true)
    local err = error.new('hello error', nil, nil, false)
    assert.match(tostring(err), 'stack traceback')
    error.debug(false)
    err = error.new('hello error', nil, nil, false)
    assert.not_match(tostring(err), 'stack traceback')
end

