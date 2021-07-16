; CALLER linkage for recv()
PUBLIC recv
EXTERN recv_callee
EXTERN ASMDISP_RECV_CALLEE

; int recv(int sockfd, void *buf, int len, int flags);
.recv
	pop hl		; return address
	pop ix		; flags (not used)
	pop bc		; length
	pop de		; buffer
	pop af		; sockfd
	push af
	push de
	push bc
	push ix
	push hl
	jp recv_callee + ASMDISP_RECV_CALLEE

