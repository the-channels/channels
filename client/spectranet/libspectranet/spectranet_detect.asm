; int __FASTCALL__ spectranet_detect(void);
PUBLIC spectranet_detect
EXTERN libspectranet

	include "spectranet.asm"
.spectranet_detect
	push bc
	ld bc,CTRLREG
	ld a,5	;cyan
	ei
	halt
	out (0xFE),a
	in a,(c)
	and 0x07	;mask off bits
	pop bc
	and a
	ld hl, 0
	cp 5
	ret z
	ld hl, -1
	ret
