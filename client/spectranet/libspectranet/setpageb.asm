; void setpageb(unsigned char page);
PUBLIC setpageb
EXTERN libspectranet
	
	include "spectranet.asm"
.setpageb
	ld a, l
	call SETPAGEB_ROM
	ret
