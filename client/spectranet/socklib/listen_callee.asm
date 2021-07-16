; process
; int listen_callee(int sockfd, int backlog);
; The Spectranet listen() implementation currently does not take a
; backlog parameter, but it must be provided for compatibility.

PUBLIC listen_callee
PUBLIC ASMDISP_LISTEN_CALLEE
	include "spectranet.asm"
.listen_callee
	pop hl		; return addr
	pop de		; int backlog
	ex (sp), hl	; swap socket/return address
	ld a, l		; socket in A
.asmentry
	HLCALL LISTEN
	jr c, err
	ld hl, 0
	ret
.err
	ld hl, -1
	ret

defc ASMDISP_LISTEN_CALLEE = asmentry - listen_callee

