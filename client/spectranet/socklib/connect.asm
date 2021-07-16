; CALLER linkage for connect()
PUBLIC connect
EXTERN connect_callee
EXTERN ASMDISP_CONNECT_CALLEE

; int connect(int sockfd, const struct sockaddr *serv_addr, socklen_t len);
.connect
	pop hl		; return addr
	pop bc		; addrlen
	pop de		; my_addr
	pop af		; sockfd
	push af
	push de
	push bc
	push hl
	jp connect_callee + ASMDISP_CONNECT_CALLEE

