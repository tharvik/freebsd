# $FreeBSD$

.include <bsd.own.mk>

LIBXOSRC=	${SRCTOP}/contrib/libxo

# Override the default suffix transformation rules for .c/.o -> .out
.SUFFIXES:
.SUFFIXES: .c .err .o .out .sh

.PATH:
.PATH.c: ${LIBXOSRC}/libxo ${LIBXOSRC}/tests/core
.PATH.err: ${LIBXOSRC}/tests/core/saved
.PATH.out: ${LIBXOSRC}/tests/core/saved

.c.out .o.out:

TESTSDIR=	${TESTSBASE}/lib/libxo

ATF_TESTS_SH+=	functional_test

BINDIR=	${TESTSDIR}

FILES+= test_01.E.err
FILES+= test_01.E.out
FILES+= test_01.H.err
FILES+= test_01.H.out
FILES+= test_01.HIPx.err
FILES+= test_01.HIPx.out
FILES+= test_01.HP.err
FILES+= test_01.HP.out
FILES+= test_01.J.err
FILES+= test_01.J.out
FILES+= test_01.JP.err
FILES+= test_01.JP.out
FILES+= test_01.T.err
FILES+= test_01.T.out
FILES+= test_01.X.err
FILES+= test_01.X.out
FILES+= test_01.XP.err
FILES+= test_01.XP.out
FILES+= test_01.err
FILES+= test_01.out
FILES+= test_02.E.err
FILES+= test_02.E.out
FILES+= test_02.H.err
FILES+= test_02.H.out
FILES+= test_02.HIPx.err
FILES+= test_02.HIPx.out
FILES+= test_02.HP.err
FILES+= test_02.HP.out
FILES+= test_02.J.err
FILES+= test_02.J.out
FILES+= test_02.JP.err
FILES+= test_02.JP.out
FILES+= test_02.T.err
FILES+= test_02.T.out
FILES+= test_02.X.err
FILES+= test_02.X.out
FILES+= test_02.XP.err
FILES+= test_02.XP.out
FILES+= test_02.err
FILES+= test_02.out
FILES+= test_03.E.err
FILES+= test_03.E.out
FILES+= test_03.H.err
FILES+= test_03.H.out
FILES+= test_03.HIPx.err
FILES+= test_03.HIPx.out
FILES+= test_03.HP.err
FILES+= test_03.HP.out
FILES+= test_03.J.err
FILES+= test_03.J.out
FILES+= test_03.JP.err
FILES+= test_03.JP.out
FILES+= test_03.T.err
FILES+= test_03.T.out
FILES+= test_03.X.err
FILES+= test_03.X.out
FILES+= test_03.XP.err
FILES+= test_03.XP.out
FILES+= test_03.err
FILES+= test_03.out
FILES+= test_04.E.err
FILES+= test_04.E.out
FILES+= test_04.H.err
FILES+= test_04.H.out
FILES+= test_04.HIPx.err
FILES+= test_04.HIPx.out
FILES+= test_04.HP.err
FILES+= test_04.HP.out
FILES+= test_04.J.err
FILES+= test_04.J.out
FILES+= test_04.JP.err
FILES+= test_04.JP.out
FILES+= test_04.T.err
FILES+= test_04.T.out
FILES+= test_04.X.err
FILES+= test_04.X.out
FILES+= test_04.XP.err
FILES+= test_04.XP.out
FILES+= test_05.E.err
FILES+= test_05.E.out
FILES+= test_05.H.err
FILES+= test_05.H.out
FILES+= test_05.HIPx.err
FILES+= test_05.HIPx.out
FILES+= test_05.HP.err
FILES+= test_05.HP.out
FILES+= test_05.J.err
FILES+= test_05.J.out
FILES+= test_05.JP.err
FILES+= test_05.JP.out
FILES+= test_05.T.err
FILES+= test_05.T.out
FILES+= test_05.X.err
FILES+= test_05.X.out
FILES+= test_05.XP.err
FILES+= test_05.XP.out
FILES+= test_06.E.err
FILES+= test_06.E.out
FILES+= test_06.H.err
FILES+= test_06.H.out
FILES+= test_06.HIPx.err
FILES+= test_06.HIPx.out
FILES+= test_06.HP.err
FILES+= test_06.HP.out
FILES+= test_06.J.err
FILES+= test_06.J.out
FILES+= test_06.JP.err
FILES+= test_06.JP.out
FILES+= test_06.T.err
FILES+= test_06.T.out
FILES+= test_06.X.err
FILES+= test_06.X.out
FILES+= test_06.XP.err
FILES+= test_06.XP.out
FILES+= test_07.E.err
FILES+= test_07.E.out
FILES+= test_07.H.err
FILES+= test_07.H.out
FILES+= test_07.HIPx.err
FILES+= test_07.HIPx.out
FILES+= test_07.HP.err
FILES+= test_07.HP.out
FILES+= test_07.J.err
FILES+= test_07.J.out
FILES+= test_07.JP.err
FILES+= test_07.JP.out
FILES+= test_07.T.err
FILES+= test_07.T.out
FILES+= test_07.X.err
FILES+= test_07.X.out
FILES+= test_07.XP.err
FILES+= test_07.XP.out
FILES+= test_08.E.err
FILES+= test_08.E.out
FILES+= test_08.H.err
FILES+= test_08.H.out
FILES+= test_08.HIPx.err
FILES+= test_08.HIPx.out
FILES+= test_08.HP.err
FILES+= test_08.HP.out
FILES+= test_08.J.err
FILES+= test_08.J.out
FILES+= test_08.JP.err
FILES+= test_08.JP.out
FILES+= test_08.T.err
FILES+= test_08.T.out
FILES+= test_08.X.err
FILES+= test_08.X.out
FILES+= test_08.XP.err
FILES+= test_08.XP.out
FILES+= test_09.E.err
FILES+= test_09.E.out
FILES+= test_09.H.err
FILES+= test_09.H.out
FILES+= test_09.HIPx.err
FILES+= test_09.HIPx.out
FILES+= test_09.HP.err
FILES+= test_09.HP.out
FILES+= test_09.J.err
FILES+= test_09.J.out
FILES+= test_09.JP.err
FILES+= test_09.JP.out
FILES+= test_09.T.err
FILES+= test_09.T.out
FILES+= test_09.X.err
FILES+= test_09.X.out
FILES+= test_09.XP.err
FILES+= test_09.XP.out
FILES+= test_10.E.err
FILES+= test_10.E.out
FILES+= test_10.H.err
FILES+= test_10.H.out
FILES+= test_10.HIPx.err
FILES+= test_10.HIPx.out
FILES+= test_10.HP.err
FILES+= test_10.HP.out
FILES+= test_10.J.err
FILES+= test_10.J.out
FILES+= test_10.JP.err
FILES+= test_10.JP.out
FILES+= test_10.T.err
FILES+= test_10.T.out
FILES+= test_10.X.err
FILES+= test_10.X.out
FILES+= test_10.XP.err
FILES+= test_10.XP.out
FILES+= test_10.err
FILES+= test_10.out
FILES+= test_11.E.err
FILES+= test_11.E.out
FILES+= test_11.H.err
FILES+= test_11.H.out
FILES+= test_11.HIPx.err
FILES+= test_11.HIPx.out
FILES+= test_11.HP.err
FILES+= test_11.HP.out
FILES+= test_11.J.err
FILES+= test_11.J.out
FILES+= test_11.JP.err
FILES+= test_11.JP.out
FILES+= test_11.T.err
FILES+= test_11.T.out
FILES+= test_11.X.err
FILES+= test_11.X.out
FILES+= test_11.XP.err
FILES+= test_11.XP.out

PROGS+= test_01
PROGS+= test_02
PROGS+= test_03
PROGS+= test_04
PROGS+= test_05
PROGS+= test_06
PROGS+= test_07
PROGS+= test_08
PROGS+= test_09
PROGS+= test_10
PROGS+= test_11

CFLAGS+=	-I${LIBXOSRC}/libxo

LIBADD=		xo

SUBDIR+=	encoder

.include <bsd.test.mk>
