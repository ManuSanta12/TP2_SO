GLOBAL cpuVendor
GLOBAL getSeconds
GLOBAL getHours
GLOBAL getMinutes
GLOBAL getTime
GLOBAL outSpeaker
GLOBAL inSpeaker
GLOBAL force_tick
section .text
	

cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

	
getSeconds:
	push rbp
	mov rbp, rsp

    mov al, 0x00
    out 70h, al
    in al, 71h

	mov rsp, rbp
	pop rbp
    ret

getMinutes:
	push rbp
	mov rbp, rsp

    mov al, 0x02
    out 70h, al
    in al, 71h

	mov rsp, rbp
	pop rbp
    ret

getHours:
	push rbp
	mov rbp, rsp

    mov al, 0x04
    out 70h, al
    in al, 71h

	mov rsp, rbp
	pop rbp
    ret		

getTime:
    push rbp
    mov rbp, rsp

    mov al, 0Bh
    out 70h, al
    in  al, 71h
    or al, 6h
    out 71h, al

    mov al, 0
    out 70h, al
    in al, 71h
    mov [rdx], al

    mov al, 2
    out 70h, al
    in al, 71h
    mov [rsi], al
    
    mov al, 4
    out 70h, al
    in al, 71h
    mov [rdi], al

    mov rsp, rbp
    pop rbp
    ret
inSpeaker:
	push rbp
	mov rbp, rsp

	mov rdx, rdi

	mov rax, 0
	in al, dx

	mov rsp, rbp
	pop rbp
	ret

outSpeaker:
	push rbp
	mov rbp, rsp 

	mov rax, rsi 
	mov rdx, rdi 
	out dx, al 

	mov rsp, rbp
	pop rbp
	ret


force_tick:
    int 0x20
    ret