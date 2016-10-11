#prefix	= $(home)
prefix	= /usr/local
bindir	= ${prefix}/bin

BIN	= ipcmod shmdump

CC	= gcc
CFLAGS	= -Wall -O2

all:	$(BIN)

ipcmod:	ipcmod.c
	$(CC) -o $@ $(CFLAGS) $<

shmdump: shmdump.c
	$(CC) -o $@ $(CFLAGS) $<

install: $(BIN)
	strip $(BIN)
	mkdir -p -m $(bindir)
	install -m 755 $(BIN) $(bindir)

clean:
	rm -f $(BIN)
