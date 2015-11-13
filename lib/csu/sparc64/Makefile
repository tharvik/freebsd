# $FreeBSD$

.PATH: ${.CURDIR}/../common

SRCS=		crt1.c crti.S crtn.S
OBJS=		${SRCS:N*.h:R:S/$/.o/g}
OBJS+=		Scrt1.o gcrt1.o
CFLAGS+=	-I${.CURDIR}/../common -I${.CURDIR}/../../libc/include

SRCS+=		${.CURDIR}/../common/safestack_tag.c
OBJS+=		safestack_tag.o

all: ${OBJS}

CLEANFILES=	${OBJS}

gcrt1.o: crt1.c
	${CC} ${CFLAGS} -DGCRT -c -o gcrt1.o ${.ALLSRC}

Scrt1.o: crt1.c
	${CC} ${CFLAGS} -fPIC -DPIC -c -o Scrt1.o ${.ALLSRC}

safestack_tag.o: ${.CURDIR}/../common/safestack_tag.c
	${CC} ${CFLAGS} -c -o safestack_tag.o ${.ALLSRC}

realinstall:
	${INSTALL} -o ${LIBOWN} -g ${LIBGRP} -m ${LIBMODE} \
	    ${OBJS} ${DESTDIR}${LIBDIR}

.include <bsd.lib.mk>
