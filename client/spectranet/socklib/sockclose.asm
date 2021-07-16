; process
; int __FASTCALL__ sockclose(int fd);
PUBLIC sockclose
EXTERN libsocket

	include "spectranet.asm"
.sockclose
	ld a, l		; file descriptor in lsb of hl
	HLCALL CLOSE
	jr c, err_close
	ld hl, 0	; return code 0
	ret
.err_close
	ld hl, -1
	ret

