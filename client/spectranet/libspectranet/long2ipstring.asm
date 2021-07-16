; CALLER linkage for long2ipstring()
PUBLIC long2ipstring
EXTERN long2ipstring_callee
EXTERN ASMDISP_LONG2IPSTRING_CALLEE

; void long2ipstring(inet_addr_t *addr, char *str);
.long2ipstring
	pop bc		; return address
	pop de		; char *str
	pop hl		; inet_addr_t *addr
	push hl
	push de
	push bc
	jp long2ipstring_callee + ASMDISP_LONG2IPSTRING_CALLEE
