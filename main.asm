; @param message to print (string)
; @param the length of the message (number)
global print
print:
	;; 1 arg
	mov	r10, rdi
	;; 2 arg
	mov	r11, rsi
	;; call write syscall
	mov	rax, 1
	mov	rdi, 1
	mov	rsi, r10
	mov	rdx, r11
	syscall
	ret
