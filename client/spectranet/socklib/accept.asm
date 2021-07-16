; CALLER linkage for accept()
PUBLIC accept
EXTERN accept_callee
EXTERN ASMDISP_ACCEPT_CALLEE

; int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
.accept
	pop hl		; return address
	pop bc		; addrlen
	pop de		; addr
	pop af		; sockfd
	push af
	push de
	push bc
	push hl
	jp accept_callee + ASMDISP_ACCEPT_CALLEE

