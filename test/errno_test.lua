local assert = require('assertex')
local testcase = require('testcase')
local errno = require('error').errno

function testcase.verify_mapping_table()
    -- verify that value of errno equals to value of errno symbol
    for k, t in pairs(errno) do
        if type(k) == 'number' then
            assert.rawequal(t, errno[t:name()])
        end
    end
end

