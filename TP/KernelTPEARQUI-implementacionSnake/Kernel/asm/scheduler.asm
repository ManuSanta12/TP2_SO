GLOBAL loadProcess
GLOBAL _int20h
GLOBAL execute_next
GLOBAL execute_from_rip
EXTERN contextSwitch

section .text

_start:
    ; Como en RCX quedo el entryPoint que estaba en rdi entonces
    ; debo llamar a este registro
    call rcx
    ; La función de userland tiene su entrypoint en rcx y va a devolver algo
    ; ese algo se guarda en rax.
    ; Guardamos el return value de la funcion en rdi para pasarselo como primer
    ; parametro a la syscall
    mov rdi, rax
    ; TODO mov rax, Numero de syscall de exit
    mov rax, 4
    int 80h

; Creamos el stack "simulado" del proceso para que el scheduler
; pueda tomar el programa y correrlo
; rdi -> entryPoint, el puntero a funcion rip
; rsi -> rsp
; rdx -> argc
; rcx -> argv

loadProcess:
   
    mov r8, rsp 	
	mov r9, rbp		
	mov rsp, rdx 	
	mov rbp, rdx
	push 0x0
	push rdx
	push 0x202
	push 0x8
	push rdi
	mov rdi, rsi 		
	mov rsi, rcx		
	pushState
	mov rax, rsp
	mov rsp, r8
	mov rbp, r9
	ret
_int20h:
    int 20h
    ret

execute_next:
    mov rsp, rdi
    popState
    iretq

execute_from_rip:
    mov rax, rdi 
    jmp rax 