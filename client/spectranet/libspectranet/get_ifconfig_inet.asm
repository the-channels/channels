; process
; void __FASTCALL__ get_ifconfig_inet(in_addr_t *addr);
PUBLIC get_ifconfig_inet
EXTERN libspectranet

	include "spectranet.asm"
.get_ifconfig_inet
	ex de, hl
	HLCALL GET_IFCONFIG_INET_ROM
	ret

