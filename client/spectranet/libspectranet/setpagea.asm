; void setpagea(unsigned char page);
PUBLIC setpagea
EXTERN libspectranet
	
	include "spectranet.asm"
.setpagea
	ld a, l
	call SETPAGEA_ROM
	ret
