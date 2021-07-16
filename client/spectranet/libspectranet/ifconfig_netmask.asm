; process
; void __FASTCALL__ ifconfig_netmask(in_addr_t *addr);
PUBLIC ifconfig_netmask
EXTERN libspectranet

	include "spectranet.asm"
.ifconfig_netmask
	IXCALL IFCONFIG_NETMASK_ROM
	ret

