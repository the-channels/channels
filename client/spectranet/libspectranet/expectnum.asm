; int expectnum
PUBLIC expectNum
EXTERN libspectranet
	
	include "zxromdefs.asm"
.expectNum
	rst 16
	defw ZX_EXPT_1NUM
	ld h, 0
	ld l, a
	ret

