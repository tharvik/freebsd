# $FreeBSD$

SSP_CFLAGS=-fno-safe-stack

SED_FIX_NOTE = -i "" -e '/\.note\.tag/s/progbits/note/'

NO_WMISSING_VARIABLE_DECLARATIONS=

.include "../Makefile.inc"
