; void exit_success()
PUBLIC exit_success
EXTERN libspectranet

	include "spectranet.asm"
.exit_success
	pop hl		; unwind return address from the stack
	jp EXIT_SUCCESS_ROM

