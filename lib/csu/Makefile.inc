# $FreeBSD$

SSP_CFLAGS=

.if !empty(CFLAGS:M-fsafe-stack)
SSP_CFLAGS=-fno-safe-stack
.endif

SED_FIX_NOTE = -i "" -e '/\.note\.tag/s/progbits/note/'

NO_WMISSING_VARIABLE_DECLARATIONS=

.include "../Makefile.inc"
