; process
; unsigned char __FASTCALL__ pollall(struct pollfd *p);
PUBLIC pollall

	include "spectranet.asm"
.pollall
	push hl			; save pointer to param
	HLCALL POLLALL_ROM
	pop ix			; use ix to fill structure
	jr z, noneready
	ld (ix+0), a		; ready sockfd
	ld h, 0
	ld l, a			; return value
	ld (ix+1), 0		; MSB always zero
	ld (ix+3), c		; flags in revents member
	ret
		
.noneready
	ld hl, 0		; return FALSE
	ret
