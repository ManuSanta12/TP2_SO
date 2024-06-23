GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt

GLOBAL interrupt_keyboard
GLOBAL interrupt_timerTick
GLOBAL interrupt_syscall
EXTERN syscall_dispatcher
EXTERN contextSwitch
GLOBAL exception_divideByZero
GLOBAL exception_invalidOpCode
GLOBAL forced_schedule
GLOBAL regdata_exc
GLOBAL inforeg
GLOBAL hasInforeg

EXTERN irqDispatcher

EXTERN timer_handler
EXTERN keyboard_handler
EXTERN exception_handler
EXTERN dv_newline
EXTERN killProcess

SECTION .text

%macro pushState 0
	push rax
	push rbx
	push rcx
	push rdx
	push rbp
	push rdi
	push rsi
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
%endmacro

%macro popState 0
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop rsi
	pop rdi
	pop rbp
	pop rdx
	pop rcx
	pop rbx
	pop rax
%endmacro

%macro endOfHardwareInterrupt 0 ; para el keyboard y el timer tick
    ; signal pic EOI (End of Interrupt)
    mov al, 20h
    out 20h, al
%endmacro

%macro saveRegistersException 0
	mov [regdata_exc+0], 	rax ;0
	mov [regdata_exc+8], 	rbx ;1
	mov [regdata_exc+16], 	rcx ;2
	mov [regdata_exc+24], 	rdx ;3
	mov [regdata_exc+32], 	rsi ;4
	mov [regdata_exc+40], 	rdi ;5
	mov [regdata_exc+48], 	rbp ;6
	mov [regdata_exc+64], 	r8  ;8
	mov [regdata_exc+72], 	r9  ;9
	mov [regdata_exc+80], 	r10 ;10
	mov [regdata_exc+88], 	r11	;11
	mov [regdata_exc+96], 	r12 ;12
	mov [regdata_exc+104], 	r13 ;13
	mov [regdata_exc+112], 	r14 ;14
	mov [regdata_exc+120], 	r15 ;15


	mov rax, rsp ; We get the value of RSP when the exception ocurred by adding the amount of pushed bytes to the current value of RSP.
	add rax, 40
	mov [regdata_exc+56], rax ;7

	mov rax, [rsp] ; We get the value of RIP when the exception ocurred by taking the interruption's return address.
	mov [regdata_exc+128], rax ;16

	mov rax, [rsp+8] ; We get the value of RFLAGS the same way (it is pushed when an interrupt occurs).
	mov [regdata_exc+136], rax ;17
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	mov rsi, rsp	; pointer a backup registros
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn





interrupt_keyboard:
	pushState

	xor rax, rax
	in al, 60h 		; 60 es el puerto del teclado AL : 8 bits
	mov rdi, rax 	; recepcion del primer parametro

	cmp al, 0x2A 	;left shift pressed
	jne .continue1
	mov byte [left_shift], 1

.continue1:
	cmp al, 0xAA 	;left shift realesed
	jne .continue2
	mov byte [left_shift], 0

	;si estan apretados SHIFT+S se copian los registros en el vector regdataDump
	;mover RSP a donde estaba antes de ser llamada la excepcion
.continue2:
	cmp byte [left_shift], 1 	; 's' pressed
	jne .continue3
	cmp al, 0x1F
	jne .continue4

	mov [inforeg+2*8], rbx
	mov [inforeg+3*8], rcx
	mov [inforeg+4*8], rdx
	mov [inforeg+5*8], rsi
	mov [inforeg+6*8], rdi
	mov [inforeg+7*8], rbp
	;ov [inforeg+8*8], rsp
	mov [inforeg+9*8], r8
	mov [inforeg+10*8], r9
	mov [inforeg+11*8], r10
	mov [inforeg+12*8], r11
	mov [inforeg+13*8], r12
	mov [inforeg+14*8], r13
	mov [inforeg+15*8], r14
	mov [inforeg+16*8], r15

	mov rax, rsp
	add rax, 160 ;120 del popstate 
	mov [inforeg+8*8], rax ;RSP

	mov rax, [rsp+15*8]
	mov [inforeg], rax ;RIP
	
	mov rax, [rsp+14*8]
	mov [inforeg+1*8], rax ;RAX

	mov byte [hasInforeg], 1

.continue3:
    call keyboard_handler

    endOfHardwareInterrupt
    popState
	iretq

.continue4:
	cmp al, 0x2E
	jne .continue3
	call killProcess
	jmp .continue3

interrupt_timerTick:
    pushState            

    call timer_handler    



    mov rdi, rsp
	call contextSwitch
	mov rsp, rax    

    endOfHardwareInterrupt
	popState              
    iretq                 


forced_schedule:
	pushState
	mov rdi, rsp
	call contextSwitch
	mov rsp, rax   
	popState
	iretq

exception_divideByZero:
	saveRegistersException

	mov rdi, 00h
	mov rsi, regdata_exc
	call exception_handler


exception_invalidOpCode:
	saveRegistersException

	mov rdi, 06h
	mov rsi, regdata_exc
	call exception_handler

interrupt_syscall:
	mov rcx, r10
	mov r9, rax
	call syscall_dispatcher
	iretq


haltcpu:
	cli
	hlt
	ret



SECTION .bss
	aux resq 1
	regdata_exc		resq	18	; reserva 18 bytes para guardar los registros para excepciones
	inforeg	resq	17	; reserve space for 17 qwords (one for each register we want to show on inforeg).
	hasInforeg 		resb 	1 	; reserve 1 byte for a boolean on whether a regdump has already occurred.
	left_shift  	resb 	1   ; shift presionado