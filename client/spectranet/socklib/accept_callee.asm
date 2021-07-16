; process
; int accept_callee(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

PUBLIC accept_callee
PUBLIC ASMDISP_ACCEPT_CALLEE
	include "spectranet.asm"
.accept_callee
	pop hl		; return addr
	pop bc		; addrlen
	pop de		; addr
	ex (sp), hl	; restore return address, fd now in l
	ld a, l
.asmentry
	ex af, af'	; save af
	ld a, d		; Is DE (struct sockaddr *addr) NULL?
	or e
	jr nz, accept_getdata
	ex af, af'	; retrieve socket fd
	HLCALL ACCEPT
	jr c, err
	ld h, 0		; success, return new socket
	ld l, a
	ret
.err
	ld hl, -1
	ret
.accept_getdata
	ex af, af'	; get sockfd back
	push de		; and the address of struct sockaddr *addr
	HLCALL ACCEPT
	jr c, err2
	ld h, 0
	ld l, a		; new socket number
	pop de		; get addr pointer back
	push hl		; save return code
	IXCALL REMOTEADDRESS
	pop hl		; retrieve return code
	ret
.err2
	pop de		; fix the stack
	ld hl, -1
	ret

defc ASMDISP_ACCEPT_CALLEE = asmentry - accept_callee
