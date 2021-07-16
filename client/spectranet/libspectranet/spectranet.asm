; Note that this version of spectranet.asm is intended for the C compiler.
; However, if your assembler doesn't like ../rom/libspectranet.asm you
; can try this one instead; it defines exactly the same values just using
; the syntax for z88dk's z80asm.
DEFC PAGEIN = 0x3FF9
DEFC PAGEOUT = 0x007C
DEFC HLCALL = 0x3FFA
DEFC IXCALL = 0x3FFD
DEFC SOCKET = 0x3E00	; Allocate a socket
DEFC CLOSE = 0x3E03	; Close a socket
DEFC LISTEN = 0x3E06	; Listen for incoming connections
DEFC ACCEPT = 0x3E09	; Accept an incoming connection
DEFC BIND = 0x3E0C	; Bind a local address to a socket
DEFC CONNECT = 0x3E0F	; Connect to a remote host
DEFC SEND = 0x3E12	; Send data 
DEFC RECV = 0x3E15	; Receive data 
DEFC SENDTO = 0x3E18	; Send data to an address
DEFC RECVFROM = 0x3E1B	; Receive data from an address
DEFC POLL_ROM = 0x3E1E	; Poll a list of sockets
DEFC POLLALL_ROM = 0x3E21	; Poll all open sockets
DEFC POLLFD_ROM = 0x3E24	; Poll a single socket
DEFC GETHOSTBYNAME_ROM = 0x3E27	; Look up a hostname
DEFC PUTCHAR42_ROM = 0x3E2A	; 42 column print write a character
DEFC PRINT42_ROM = 0x3E2D	; 42 column print a null terminated string
DEFC CLEAR42_ROM = 0x3E30	; Clear the screen and reset 42-col print
DEFC SETPAGEA_ROM = 0x3E33	; Sets page area A
DEFC SETPAGEB_ROM = 0x3E36	; Sets page area B
DEFC LONG2IPSTRING_ROM = 0x3E39	; Convert a 4 byte big endian long to an IP
DEFC IPSTRING2LONG_ROM = 0x3E3C	; Convert an IP to a 4 byte big endian long
DEFC RAND16_ROM = 0x3E42	; 16 bit PRNG
DEFC REMOTEADDRESS = 0x3E45	; Fill struct sockaddr_in
DEFC IFCONFIG_INET_ROM = 0x3E48	; set inet4 address
DEFC IFCONFIG_NETMASK_ROM = 0x3E4B	; Set netmask
DEFC IFCONFIG_GW_ROM = 0x3E4E	; Set gateway
DEFC SETHWADDR_ROM = 0x3E51	; Set the MAC address
DEFC GETHWADDR_ROM = 0x3E54	; Read the MAC address
DEFC DECONFIG_ROM = 0x3E57	; Deconfigure inet, netmask and gateway
DEFC MAC2STRING_ROM = 0x3E5A	; Convert 6 byte MAC address to a string
DEFC STRING2MAC_ROM = 0x3E5D	; Convert a hex string to a 6 byte MAC address
DEFC INPUTSTRING_ROM = 0x3E6C	; Read a string into buffer at DE
DEFC GET_IFCONFIG_INET_ROM = 0x3E6F	; Gets the current IPv4 address
DEFC GET_IFCONFIG_NETMASK_ROM = 0x3E72	; Gets the current netmask
DEFC GET_IFCONFIG_GW_ROM = 0x3E75	; Gets the current gateway address
DEFC SETTRAP_ROM = 0x3E78	; Sets the programmable trap
DEFC DISABLETRAP_ROM = 0x3E7B	; Disables the programmable trap
DEFC ENABLETRAP_ROM = 0x3E7E	; Enables the programmable trap
DEFC PUSHPAGEA_ROM = 0x3E81	; Pages a page into area A, pushing the old one
DEFC POPPAGEA_ROM = 0x3E84	; Restores the previous page in area A
DEFC PUSHPAGEB_ROM = 0x3E87	; Pages into area B pushing the old one
DEFC POPPAGEB_ROM = 0x3E8A	; Restores the previous page in area B
DEFC PAGETRAPRETURN_ROM = 0x3E8D ; Returns from a trap to page area B
DEFC TRAPRETURN_ROM = 0x3E90	; Returns from a trap that didn't page area B
DEFC ADDBASICEXT_ROM = 0x3E93	; Adds a BASIC command
DEFC STATEMENT_END_ROM = 0x3E96	; Check for statement end, exit at syntax time
DEFC EXIT_SUCCESS_ROM = 0x3E99	; Use this to exit successfully after cmd

DEFC POLLNVAL = 0x80

; Port defines
DEFC CTRLREG = 0x033B
DEFC CPLDINFO = 0x023B