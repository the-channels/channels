; process
; void deconfig();

PUBLIC deconfig
EXTERN libspectranet
	include "spectranet.asm"
.deconfig
	HLCALL DECONFIG_ROM
	ret

