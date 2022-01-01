TARGET=$(PACKAGE).$(LIB_EXTENSION)
VARS=$(wildcard $(VARDIR)/*.txt)
SRCS=$(wildcard $(SRCDIR)/*.c)
OBJS=$(SRCS:.c=.o)
INSTALL?=install

.PHONY: all install clean

all:  $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) $(CPPFLAGS) -o $@ -c $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS) $(LIBS) $(PLATFORM_LDFLAGS)

install:
	$(INSTALL) $(TARGET) $(LIBDIR)
	$(INSTALL) src/lua_error.h $(LUA_INCDIR)
	rm -f ./src/*.o
	rm -f ./*.so

clean:
	rm -f ./src/*.o
	rm -f ./*.so
