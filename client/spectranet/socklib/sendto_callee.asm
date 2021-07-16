; process
; int sendto_callee(int sockfd, const void *buf, int len, int flags,
;                   const struct sockaddr *to, socklen_t tolen);
PUBLIC sendto_callee
PUBLIC ASMDISP_SENDTO_CALLEE
	include "spectranet.asm"

.sendto_callee
	pop hl			; return address
	pop af			; tolen is not used
	pop de			; struct sockaddr *to
	ld (_sendtobufptr), de	; save it
	pop af			; flags not used
	pop bc			; int len
	pop de			; const void *buf
	ex (sp), hl		; swap return address and int sockfd
	ld a, l			; sockfd in A
	ld hl, (_sendtobufptr)
.asmentry
	push de

	; set up internal socket library sockinfo structure.
	; Possible TODO: make struct sockaddr_in the same order as sockinfo
	; to avoid this.
	inc hl			; advance past int sin_family
	inc hl
	push bc
	ld de, _sendtoremoteport
	ldi			; sin_port
	ldi
	ld de, _sendtoremoteip
	ldi			; sin_addr
	ldi
	ldi
	ldi
	pop bc
	ld hl, 0
	ld (_sendtolocport), hl
	pop de

	ld hl, _sendtoremoteip	; hl = start of sockinfo
	IXCALL SENDTO
	jr c, sendto_err
	ld h, b			; bytes sent in BC, transfer to HL
	ld l, c
	ret
	
.sendto_err
	ld hl, -1
	ret

defc ASMDISP_SENDTO_CALLEE = asmentry - sendto_callee

._sendtobufptr	defs 2		; temporary storage for buffer ptr
._sendtoremoteip defs 4		; remote IP
._sendtoremoteport defs 2	; remote port
._sendtolocport defs 2		; local port
