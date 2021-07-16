; process
; int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
; Bind a name to a local address
; This is simplified compared to the full BSD implementation; the Spectranet
; only uses the port address (and sockaddr_in is the only type of struct
; sockaddr that's actually defined).

PUBLIC bind_callee
PUBLIC ASMDISP_BIND_CALLEE
	include "spectranet.asm"
.bind_callee
	pop hl		; return addr
	pop bc		; addrlen
	pop ix		; my_addr structure
	ex (sp), hl	; restore return addr, fd now in l
	ld a, l
.asmentry
	ld e, (ix+2)	; sin_port LSB
	ld d, (ix+3)	; sin_port MSB
	HLCALL BIND
	jr c, err
	ld hl, 0	; return code 0
	ret
.err
	ld h, 0x80	; -ve
	ld l, a		; return code
	ret

defc ASMDISP_BIND_CALLEE = asmentry - bind_callee

