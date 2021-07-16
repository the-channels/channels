; int find_int2();
; note: call only when Spectranet memory is paged in.
PUBLIC find_int2
EXTERN libspectranet

	include "zxromdefs.asm"
.find_int2
	rst 16			; CALLBAS
	defw ZX_FIND_INT2	; get 16 bit integer
	ld h, b
	ld l, c
	ret

