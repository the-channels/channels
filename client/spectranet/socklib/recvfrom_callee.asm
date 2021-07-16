; process
; int recvfrom_callee(int sockfd, void *buf, size_t len, int flags, 
;                     struct sockaddr *from, socklen_t *fromlen);
PUBLIC recvfrom_callee
PUBLIC ASMDISP_RECVFROM_CALLEE
	include "spectranet.asm"
.recvfrom_callee
	ld ix, 2
	add ix, sp		; ix = socklen_t *fromlen
	ld a, (ix+10)		; get sockfd
	pop de			; return address
	ld hl, 12
	add hl, sp		; hl = new address for return
	ld sp, hl		; set new stack pointer
	push de			; re-stack return address
.asmentry
	ld e, (ix+8)		; lsb of void *buf 
	ld d, (ix+9)		; msb
	ld c, (ix+6)		; lsb of size_t len
	ld b, (ix+7) 		; msb
	ld l, (ix+2)		; lsb of struct sockaddr *
	ld h, (ix+3)
	push hl
	ld hl, _recvfromremoteip
	push hl
	IXCALL RECVFROM
	pop hl
	jr c, recvfrom_err

	; Copy sockinfo buffer into a struct sockaddr_in.
	; Possible TODO: change sockaddr_in to match the layout of
	; the sockinfo buffer.
	pop de			; struct sockaddr *
	ld a, d			; test for NULL
	or e
	jr z, nofill
	push bc
	ld hl, _recvfromremoteport
	inc de
	inc de			; go past sin_family
	ldi			; copy port number
	ldi
	ld hl, _recvfromremoteip
	ldi			; copy IP address
	ldi
	ldi
	ldi
	pop bc
.nofill
	ld h, b			; bytes received in BC
	ld l, c
	ret
.recvfrom_err
	ld hl, -1
	ret

defc ASMDISP_RECVFROM_CALLEE = asmentry - recvfrom_callee

._recvfromremoteip	defs 4	; temporary sockinfo buffer
._recvfromremoteport	defs 2	; remote port
._recvfromlocalport	defs 2	; local port

