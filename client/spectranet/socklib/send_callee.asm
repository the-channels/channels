; process
; int send_callee(int sockfd, const void *buf, int len, int flags);
; The flags field is not currently used by the Spectranet implementation
; of send, but it must be provided for compatibility.

PUBLIC send_callee
PUBLIC ASMDISP_SEND_CALLEE
	include "spectranet.asm"
.send_callee
	pop hl		; return addr
	pop ix		; flags
	pop bc		; length
	pop de		; buffer address
	ex (sp), hl	; restore return address, sock in l
	ld a, l		; sock in a
.asmentry
	HLCALL SEND
	jr c, err	; send failed?
	
	; bytes sent value in BC, move it to hl
	ld h, b
	ld l, c
	ret
.err
	ld hl, -1	; error return - todo: set errno
	ret

defc ASMDISP_SEND_CALLEE = asmentry - send_callee
