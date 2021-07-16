; CALLER linkage for listen()
PUBLIC listen
EXTERN listen_callee
EXTERN ASMDISP_LISTEN_CALLEE

; int listen(int sockfd, int backlog);
.listen
	pop hl		; return address
	pop de		; backlog
	pop af		; sockfd
	push af
	push de
	push hl
	jp listen_callee + ASMDISP_LISTEN_CALLEE

