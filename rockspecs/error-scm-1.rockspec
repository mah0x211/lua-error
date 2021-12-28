rockspec_format = '3.0'
package = 'error'
version = 'scm-1'
source = {
    url = 'git+https://github.com/mah0x211/lua-error.git'
}
description = {
    summary = 'additional features to the error module.',
    homepage = 'https://github.com/mah0x211/lua-error',
    license = 'MIT/X11',
    maintainer = 'Masatoshi Fukunaga'
}
dependencies = {
    'lua >= 5.1',
}
build = {
    type = 'make',
    build_variables = {
        CFLAGS          = '$(CFLAGS)',
        WARNINGS        = '-Wall -Wno-trigraphs -Wmissing-field-initializers -Wreturn-type -Wmissing-braces -Wparentheses -Wno-switch -Wunused-function -Wunused-label -Wunused-parameter -Wunused-variable -Wunused-value -Wuninitialized -Wunknown-pragmas -Wshadow -Wsign-compare',
        CPPFLAGS        = '-I$(LUA_INCDIR) -Ideps/lauxhlib',
        LDFLAGS         = '$(LIBFLAG)',
        LIB_EXTENSION   = '$(LIB_EXTENSION)'
    },
    install_variables = {
        LIB_EXTENSION   = '$(LIB_EXTENSION)',
        PREFIX          = '$(PREFIX)',
        BINDIR          = '$(BINDIR)',
        CONFDIR         = '$(CONFDIR)',
        LUA_INCDIR      = '$(LUA_INCDIR)',
        INST_LIBDIR     = '$(LIBDIR)',
    }
}
