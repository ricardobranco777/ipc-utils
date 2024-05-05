BIN	= ipcmk
BINDIR	= /usr/local/bin/

CC	= gcc
CFLAGS	= -Wall -O2

OBJS	= expand_number.o

.PHONY: all
all:	$(BIN)

$(BIN):	$(BIN).c $(OBJS)
	$(CC) -o $@ $(CFLAGS) $< $(OBJS)

.PHONY: install
install: $(BIN)
	@install -s -m 755 $(BIN) $(BINDIR)

.PHONY: uninstall
uninstall:
	@rm -f $(BINDIR)/$(BIN)

.PHONY: clean
clean:
	@rm -f $(BIN) *.o
