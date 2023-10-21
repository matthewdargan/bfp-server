TARGET = bfp_server
OBJS = bfp_server.o
CFLAGS += -W -Wall -Wshadow -Werror -pedantic -g -std=c11
MANUAL = bfp_server.1

PREFIX ?= /usr/local
BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man/man1

INSTALL ?= install

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET)

bfp_server.o: bfp_server.c version.h

.PHONY: install
install:
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -m 0755 $(TARGET) $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)
	$(INSTALL) -m 0644 $(MANUAL) $(DESTDIR)$(MANDIR)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)