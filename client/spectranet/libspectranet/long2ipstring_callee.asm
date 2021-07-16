; process
; CALLEE linkage for long2ipstring
; int long2ipstring_callee(inet_addr_t *addr, char *str);

PUBLIC long2ipstring_callee
PUBLIC ASMDISP_LONG2IPSTRING_CALLEE
	include "spectranet.asm"
.long2ipstring_callee
	pop bc		; return address
	pop de		; char *str
	pop hl		; inet_addr_t *addr
	push bc		; restore return address
.asmentry
	IXCALL LONG2IPSTRING_ROM
	ret

defc ASMDISP_LONG2IPSTRING_CALLEE = asmentry - long2ipstring_callee

