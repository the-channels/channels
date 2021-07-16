; process
; void __FASTCALL__ ifconfig_gw(in_addr_t *addr);
PUBLIC ifconfig_gw
EXTERN libspectranet

	include "spectranet.asm"
.ifconfig_gw
	IXCALL IFCONFIG_GW_ROM
	ret

