-- helper module for test
local stdout = io.stdout
local format = require('string.format.all')
local function printf(...)
    stdout:write(format(...))
end

local traceback = debug.traceback

local function runner(testcase)
    local before_all = testcase.before_all
    local before_each = testcase.before_each
    local after_all = testcase.after_all
    local after_each = testcase.after_each
    printf('Run %d tests\n', #testcase)

    local nerr = 0
    if before_all then
        before_all()
    end

    for _, v in ipairs(testcase) do
        printf('- %s ... ', v.name)
        if before_each then
            before_each()
        end

        do
            local ok, err = xpcall(v.func, traceback)
            if ok then
                print('ok')
            else
                print('failed')
                print(err)
                nerr = nerr + 1
            end
        end

        if after_each then
            after_each()
        end
        collectgarbage('collect')
    end

    if after_all then
        after_all()
    end

    print('================================')
    if nerr < 1 then
        printf('All %d tests passed\n', #testcase)
        return
    end

    error(format('%d tests succeeded; %d failed', #testcase - nerr, nerr))
end

--- new_testcase creates new testcase
--- @return table testcase
--- @return any err
local function new()
    local pre_post_process = {
        before_all = true,
        before_each = true,
        after_all = true,
        after_each = true,
    }
    local testcase = {}
    return setmetatable({}, {
        __call = function()
            runner(testcase)
        end,
        __newindex = function(_, name, func)
            assert(type(name) == 'string', 'name must be string')
            assert(type(func) == 'function', 'func must be function')
            if testcase[name] then
                error('testcase ' .. name .. ' already exists')
            elseif not pre_post_process[name] then
                testcase[#testcase + 1] = {
                    name = name,
                    func = func,
                }
            end
            testcase[name] = func
        end,
    })
end

return new

