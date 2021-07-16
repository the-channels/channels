; CALLER linkage for bind()
PUBLIC bind
EXTERN bind_callee
EXTERN ASMDISP_BIND_CALLEE

; int bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen);
.bind
	pop hl		; return addr
	pop bc		; addrlen
	pop ix		; my_addr
	pop af		; sockfd
	push af
	push ix
	push bc
	push hl
	jp bind_callee + ASMDISP_BIND_CALLEE

