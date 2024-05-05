OPSYS:sh = uname

PROG= ipcmk
WARNS= 3
CFLAGS+= -O2
MK_DEBUG_FILES= no
SRCS= ipcmk.c
MAN=
.if ${OPSYS} == "FreeBSD"
LDFLAGS= -lutil
.else
SRCS+= expand_number.c
.endif
HDRS= extern.h

.include <bsd.prog.mk>
