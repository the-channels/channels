; process
; unsigned char __FASTCALL__ pollfd(int sockfd);
PUBLIC poll_fd

	include "spectranet.asm"
.poll_fd
	ld a, l			; get fd
	HLCALL POLLFD_ROM
	ld h, 0			; h should always be cleared
	jr c, err_pollfd
	ld l, c			; flags in C
	ret
.err_pollfd
	ld l, POLLNVAL		; error return code
	ret

