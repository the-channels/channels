; next_char() - returns the next character on the BASIC line
PUBLIC next_char
EXTERN libspectranet

	include "zxromdefs.asm"
.next_char
	rst 16
	defw ZX_NEXT_CHAR
	ld h, 0
	ld l, a
	ret

