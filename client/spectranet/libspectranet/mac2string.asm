; CALLER linkage for mac2string
; void mac2string(char *mac, char *str);

PUBLIC mac2string
EXTERN mac2string_callee
EXTERN ASMDISP_MAC2STRING_CALLEE

.mac2string
	pop bc		; ret addr
	pop de		; char *str
	pop hl		; char *mac
	push hl
	push de
	push bc
	jp mac2string_callee + ASMDISP_MAC2STRING_CALLEE

