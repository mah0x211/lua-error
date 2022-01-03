local concat = table.concat
local format = string.format
local gsub = string.gsub
local match = string.match
local upper = string.upper
local lower = string.lower

local function var_files(...)
    local files = {}

    for _, pathname in ipairs({
        ...,
    }) do
        local name = match(pathname, '^[^%s]+/([_%w]+)%.txt$')
        if name then
            files[name] = pathname
        end
    end

    return files
end

local function tmpl_files(...)
    local files = {}

    for _, pathname in ipairs({
        ...,
    }) do
        local basename = match(pathname, '^[^%s]+/([_%w]+%.c)$')
        if basename then
            files[basename] = pathname
        end
    end

    return files, files
end

local function codegen(vars)
    local TMPL = [=[
#ifdef __DEFVAL__
    lua_pushliteral(L, "__DEFVAL__");
    lua_pushliteral(L, "__DEFVAL__");
    le_new_type(L, -1);
    lua_pushvalue(L, -1);
    lua_rawseti(L, -4, __DEFVAL__);
    lua_rawset(L, -3);
#endif
]=]
    local decls = {}

    for name, pathname in pairs(vars) do
        local f = assert(io.open(pathname))
        local decl = {}

        for line in f:lines() do
            local symbol = line:match('^[_%w]+$')

            if not symbol then
                error('invalid line: ' .. line)
            elseif not decl[symbol] then
                decl[symbol] = gsub(TMPL, '__DEFVAL__', symbol)
                -- keep symbol for sort
                decl[#decl + 1] = symbol
            end
        end

        -- generate
        table.sort(decl)
        for i, symbol in ipairs(decl) do
            decl[i], decl[symbol] = decl[symbol], nil
        end
        decls[upper(name)] = concat(decl, '\n')

        f:close()
    end

    return decls
end

local function genfile(tmpl, decls)
    for basename, pathname in pairs(tmpl) do
        local file = assert(io.open(pathname)):read('*a')
        local replace = function(def, name)
            local code = decls[name]

            if not code then
                return format(
                           '\n#error "cannot generate %s: var/%s.txt not found"\n',
                           def, lower(name))
            end

            return '\n' .. code
        end

        file = gsub(file, '\n%s*#define (GENDECL_([_%w]+))\n', replace)
        assert(io.open('src/' .. basename, 'w')):write(file)
    end
end
-- io.open('./signal.c', 'w'):write(file)

local vars = var_files(...)
local tmpl = tmpl_files(...)
local decls = codegen(vars)
genfile(tmpl, decls)
