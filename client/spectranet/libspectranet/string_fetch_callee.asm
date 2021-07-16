; callee linkage for string_fetch
; unsigned int string_fetch(char *buf, int bufsz);
PUBLIC string_fetch_callee
GLOBAL ASMDISP_STRING_FETCH_CALLEE

	include "zxromdefs.asm"
.string_fetch_callee
	rst 16		; CALLBAS
	defw ZX_STK_FETCH ; fetch string expression - start in bc, length in de
	ld a, b
	or c		; zero length string?
	jr z, zerolen
	ld ix, 2
	add ix, sp	; point ix at the length arg
	ld a, (ix+1)	; msb of dest buffer size
	cp b		; msb of src buffer size
	jr c, setlen	; dest is definitely smaller than the source
	ld a, c		; compare the returned length
	cp (ix+0)	; with the size of the buffer
	jr c, copybuf	; copy the buffer if buffer is larger than source
.setlen
	ld c, (ix+0)	; set length to copy to passed in length
	ld b, (ix+1)
	dec bc		; leaving room for the NULL
.copybuf
	ex de, hl	; source addr to hl
	ld e, (ix+2)	; destination address
	ld d, (ix+3)
	push bc		; save byte count
	ldir		; copy the buffer
	pop bc
	ex de, hl	; get end of buffer
	ld (hl), 0	; put a NULL on the end

	pop hl		; unwind the stack - get sp
	pop de		; remove length param
	ex (sp), hl	; put return address back in its rightful place
	ld h, b		; return number of bytes copied
	ld l, c
	ret
.zerolen
	ld l, (ix+2)
	ld h, (ix+3)
	ld (hl), 0	; NULL in the first byte of the buffer
	ld hl, 0	; return 0 length
	ret

;defc ASMDISP_STRING_FETCH_CALLEE = asmentry . string_fetch_callee

