local stdout = io.stdout
local format = require('string.format.all')
local function printf(...)
    stdout:write(format(...))
end

print('================================')
print('RUN TEST FILES')
print('================================')
print('')
local errors = {}
for _, filename in ipairs({
    'test/check_test.lua',
    'test/error_test.lua',
    'test/error_type_test.lua',
    'test/is_test.lua',
    'test/fatalf_test.lua',
    'test/format_test.lua',
    'test/message_test.lua',
}) do
    printf('@%s: ', filename)
    local ok, err = xpcall(function()
        dofile(filename)
    end, debug.traceback)
    if not ok then
        print('ERROR: failed to run test')
        errors[#errors + 1] = {
            filename = filename,
            err = err,
        }
    end
    print('\n')
end

if #errors < 1 then
    return
end

print('================================')
print('FAILED TO RUN TEST FILES')
print('================================')
for _, v in ipairs(errors) do
    printf('- %s\n', v.filename)
    print(v.err)
end
print('')
os.exit(1)
