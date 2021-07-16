; process
; void __FASTCALL__ ifconfig_inet(in_addr_t *addr);
PUBLIC ifconfig_inet
EXTERN libspectranet

	include "spectranet.asm"
.ifconfig_inet
	IXCALL IFCONFIG_INET_ROM
	ret

