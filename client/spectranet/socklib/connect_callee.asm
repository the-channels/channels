; process
; int connect(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
; Connect to a remote address
; This is simplified compared to the full BSD implementation; the Spectranet
; only is aware of inet sockets (so this only handles struct sockaddr_in)

PUBLIC connect_callee
PUBLIC ASMDISP_CONNECT_CALLEE
	include "spectranet.asm"
.connect_callee
	pop hl		; return addr
	pop bc		; addrlen
	pop de		; serv_addr structure
	ex (sp), hl	; restore return addr, sockfd in L
	ld a, l
.asmentry
	ex de, hl	; serv_addr pointer now in hl
	inc hl
	inc hl		; (hl) now points at port LSB
	ld c, (hl)	; sin_port LSB
	inc hl
	ld b, (hl)	; sin_port MSB
	inc hl
	ex de, hl	; de now points at address
	HLCALL CONNECT
	jr c, err
	ld hl, 0	; return 0
	ret
.err
	ld h, 0x80	; TODO - set errno
	ld l, a
	ret

defc ASMDISP_CONNECT_CALLEE = asmentry - connect_callee

