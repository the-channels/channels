; process
; int __FASTCALL__ addbasicext(struct basic_cmd *cmd);
PUBLIC addbasicext
EXTERN libspectranet

	include "spectranet.asm"
.addbasicext
	IXCALL ADDBASICEXT_ROM
	ld hl, 0
	ret nc
	dec hl
	ret

