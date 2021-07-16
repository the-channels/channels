; process
; int recv_callee(int sockfd, const void *buf, int len, int flags);
; The flags field is not currently used by the Spectranet implementation
; of recv, but it must be provided for compatibility.

PUBLIC recv_callee
PUBLIC ASMDISP_RECV_CALLEE
	include "spectranet.asm"
.recv_callee
	pop hl		; return addr
	pop ix		; flags
	pop bc		; length of buffer
	pop de		; buffer address
	ex (sp), hl	; restore return address, sock in l
	ld a, l		; sock in a
.asmentry
	HLCALL RECV
	jr c, err	; recv failed?
	
	; bytes sent value in BC, move it to hl
	ld h, b
	ld l, c
	ret
.err
	ld hl, -1	; error return - todo: set errno
	ret

defc ASMDISP_RECV_CALLEE = asmentry - recv_callee

