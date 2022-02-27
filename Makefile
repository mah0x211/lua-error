TARGET=$(PACKAGE).$(LIB_EXTENSION)
VARS=$(wildcard var/*.txt)
TMPL=$(wildcard tmpl/*.c)
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:.c=.o)
INSTALL?=install

.PHONY: preprocess all install clean

all: preprocess $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(COVERAGE) $(CPPFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS) $(COVERAGE)

preprocess:
	lua ./codegen.lua $(VARS) $(TMPL)

install:
	$(INSTALL) $(TARGET) $(LIBDIR)
	$(INSTALL) src/lua_error.h $(LUA_INCDIR)
	rm -f ./src/*.o
	rm -f ./*.so

clean:
	rm -f ./src/*.o
	rm -f ./*.so
