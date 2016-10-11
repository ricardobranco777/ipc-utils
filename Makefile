#prefix	= $(home)
prefix	= /usr/local
bindir	= ${prefix}/bin

BIN	= ipcmod shmdump
OBJ	= lib.o

CC	= gcc
CFLAGS	= -Wall -O2

all:	$(BIN)

ipcmod:	ipcmod.c $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $<

shmdump: shmdump.c $(OBJ)
	$(CC) -o $@ $(CFLAGS) $(OBJ) $<

lib.o:	lib.c
	$(CC) -o $@ -c $(CFLAGS) $<

install: $(BIN)
	strip $(BIN)
	mkdir -p -m $(bindir)
	install -m 755 $(BIN) $(bindir)

clean:
	rm -f $(BIN) $(OBJ)
