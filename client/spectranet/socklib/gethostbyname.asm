; process
; struct hostent * __FASTCALL__ gethostbyname(const char *name);
PUBLIC gethostbyname
PUBLIC _h_errno

	include "spectranet.asm"
.gethostbyname
	; hl already contains the pointer to the string. DE must point
	; at our static storage.
	ld de, _he_addr1
	IXCALL GETHOSTBYNAME_ROM
	jr c, err
	ld hl, _he_addr1
	ld (_he_h_addr_list), hl	; set addr list pointer

	ld b, 4
	ld hl, _he_addr2	; TODO: make this dynamic	
.nullterm
	ld (hl), 0
	inc hl
	djnz nullterm

	; TODO: return official hostname
	ld hl, 0
	ld (_he_hostname), hl
	
	ld hl, _he_hostname	; start of struct - pointer to return
	ret
.err
	ld h, 0
	ld l, a			; error number
	ld (_h_errno), hl
	ld hl, 0		; return a NULL pointer
	ret

; Static variables
._h_errno	defs 2
._he_hostname	defs 2		; char *hostname in struct hostent
._he_h_addr_list defs 2		; unsigned long *h_addr_list
._he_addr1	defs 4		; first address
._he_addr2	defs 4		; second address

