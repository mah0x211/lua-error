SRCS=$(wildcard src/*.c)
CLIBS=$(SRCS:.c=.$(LIB_EXTENSION))
GCDAS=$(SRCS:.c=.gcda)
LUALIBS=$(wildcard lib/*.lua)
INSTALL?=install

ifdef ERROR_COVERAGE
COVFLAGS=--coverage
endif

.PHONY: all install

all: $(CLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(COVFLAGS) $(CPPFLAGS) -o $@ -c $<

%.$(LIB_EXTENSION): %.o
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVFLAGS)

install:
	$(INSTALL) error.lua $(INST_LUADIR)
	$(INSTALL) -d $(INST_LIBDIR)
	$(INSTALL) $(LUALIBS) $(INST_LIBDIR)
	$(INSTALL) -d $(INST_CLIBDIR)
	$(INSTALL) $(CLIBS) $(INST_CLIBDIR)
	$(INSTALL) src/lua_error.h $(LUA_INCDIR)
	rm -f $(CLIBS) $(GCDAS)
