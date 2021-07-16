; int expect2Num
PUBLIC expect2Num
EXTERN libspectranet

	include "zxromdefs.asm"
.expect2Num
	rst 16
	defw ZX_NEXT_2NUM
	ld h, 0
	ld l, a
	ret

