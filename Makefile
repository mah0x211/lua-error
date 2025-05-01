SRCS=$(wildcard src/*.c)
CLIBS=$(SRCS:.c=.$(LIB_EXTENSION))
GCDAS=$(SRCS:.c=.gcda)
LUALIBS=$(wildcard lib/*.lua)
INSTALL?=install
CCX:=$(CC)
MAKE_TARGET:=$(addprefix -object ,$(shell cat .make-target))

ifdef ERROR_COVERAGE
CCX:=$(subst gcc,clang,$(CC))
COVFLAGS=-fprofile-instr-generate -fcoverage-mapping
endif

.PHONY: all install clean test coverage

all: clean $(CLIBS)
	@echo "Exporting $(CLIBS) to .make-target"
	@echo $(CLIBS) > .make-target

%.o: %.c
	$(CCX) $(CFLAGS) $(WARNINGS) $(COVFLAGS) $(CPPFLAGS) -o $@ -c $<

%.$(LIB_EXTENSION): %.o
	$(CCX) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVFLAGS)

install:
	$(INSTALL) error.lua $(INST_LUADIR)
	$(INSTALL) -d $(INST_LIBDIR)
	$(INSTALL) $(LUALIBS) $(INST_LIBDIR)
	$(INSTALL) -d $(INST_CLIBDIR)
	$(INSTALL) $(CLIBS) $(INST_CLIBDIR)
	$(INSTALL) src/lua_error.h $(LUA_INCDIR)
	rm -f $(GCDAS)

clean:
	rm -f $(GCDAS) $(CLIBS)

test:
	@echo "Cleaning up coverage data..."
	rm -f profiles/*.profraw
	@echo "Running tests..."
	LLVM_PROFILE_FILE=profiles/%p-%m.profraw lua ./test/testall.lua

coverage: test
	llvm-profdata merge -sparse profiles/*.profraw -o default.profdata && \
  	llvm-cov export $(MAKE_TARGET) \
		-instr-profile=default.profdata \
  		-ignore-filename-regex='.+/include/.*' \
  		-ignore-filename-regex='.+/deps/.*' \
  		-format=lcov > lcov.info
	llvm-cov report $(MAKE_TARGET) \
		-instr-profile=default.profdata \
		-ignore-filename-regex='.+/include/.*' \
  		-ignore-filename-regex='.+/deps/.*'
