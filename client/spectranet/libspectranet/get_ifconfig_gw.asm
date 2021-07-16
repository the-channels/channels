; process
; void __FASTCALL__ get_ifconfig_gw(in_addr_t *addr);
PUBLIC get_ifconfig_gw
EXTERN libspectranet

	include "spectranet.asm"
.get_ifconfig_gw
	ex de, hl
	HLCALL GET_IFCONFIG_GW_ROM
	ret

