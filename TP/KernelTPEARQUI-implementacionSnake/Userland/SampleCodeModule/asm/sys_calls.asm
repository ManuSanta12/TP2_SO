GLOBAL sys_read
GLOBAL sys_write
GLOBAL sys_clear
GLOBAL sys_getMinutes
GLOBAL sys_getHours
GLOBAL sys_getSeconds
GLOBAL sys_scrHeight
GLOBAL sys_scrWidth
GLOBAL sys_fillRect
GLOBAL sys_wait
GLOBAL sys_inforeg
GLOBAL sys_printmem
GLOBAL sys_pixelMinus
GLOBAL sys_pixelPlus
GLOBAL sys_playSound
GLOBAL sys_mute
GLOBAL sys_mem
GLOBAL sys_memInfo
GLOBAL sys_memMalloc
GLOBAL sys_memFree
GLOBAL sys_semInit
GLOBAL sys_semPost
GLOBAL sys_semWait
GLOBAL sys_newProcess
GLOBAL sys_getPid
GLOBAL sys_semClose
GLOBAL sys_sleepTime
GLOBAL sys_nice
GLOBAL sys_pipe
GLOBAL sys_dup2
GLOBAL sys_open
GLOBAL sys_close
GLOBAL sys_ps
GLOBAL sys_changeProcessStatus
GLOBAL sys_getCurrentPid
GLOBAL sys_exec
GLOBAL sys_exit
GLOBAL sys_waitpid
GLOBAL sys_kill
GLOBAL sys_block
GLOBAL sys_unblock
GLOBAL sys_getPriority
section .text

; Pasaje de parametros en C:
; %rdi %rsi %rdx %rcx %r8 %r9

; Pasaje de parametros para sys_calls
; %rdi %rsi %rdx %r10 %r8 %r9

; MOVER RCX a R10

; %rcx y %rc11 son destruidos si las llamo desde aca

; %rax el numero de la syscall

sys_read:
    mov rax, 0x00
    int 80h
    ret

sys_write:
    mov rax, 0x01
    int 80h
    ret

sys_clear:
    mov rax, 0x02
    int 80h
    ret

sys_getHours:
    mov rax, 0x03
    int 80h
    ret

sys_getMinutes:
    mov rax, 0x04
    int 80h
    ret

sys_getSeconds:
    mov rax, 0x05
    int 80h
    ret

sys_scrHeight:
    mov rax, 0x06
    int 80h
    ret

sys_scrWidth:
    mov rax, 0x07
    int 80h
    ret

sys_fillRect:
    mov rax, 0x08
    mov r10, rcx        ;4to parametro de syscall es R10
    int 80h
    ret

sys_wait:
    mov rax, 0x09
    int 80h
    ret

sys_inforeg:
    mov rax, 0x0A
    int 80h
    ret

sys_pixelPlus: 
    mov rax, 0x0B
    int 80h
    ret

sys_pixelMinus: 
    mov rax, 0x0C
    int 80h
    ret

sys_playSound: 
    mov rax, 0x0D
    int 80h
    ret

sys_mute: 
    mov rax, 0x0E
    int 80h
    ret
sys_memInfo:
    mov rax, 0x0F
    int 80h
    ret

sys_memMalloc:
    mov rax, 0x10
    int 80h
    ret

sys_memFree:
    mov rax, 0x11
    int 80h
    ret

sys_semInit:
    mov rax, 0x12
    int 80h
    ret
sys_semPost:
    mov rax, 0x13
    int 80h
    ret
sys_semWait:
    mov rax, 0x14
    int 80h
    ret
sys_newProcess:
    mov rax, 0x15
    int 80h
    ret
sys_getPid:
    mov rax, 0x16
    int 80h
    ret

sys_semClose:
    mov rax, 0x17
    int 80h
    ret

sys_sleepTime:
    mov rax, 0x18
    int 80h
    ret

sys_nice:
    mov rax, 0x19
    int 80h
    ret

sys_pipe:
    mov rax, 0x1A
    int 80h
    ret

sys_dup2:
    mov rax, 0x1B
    int 80h
    ret

sys_open:
    mov rax, 0x1C
    int 80h
    ret

sys_close:
    mov rax, 0x1D
    int 80h
    ret

sys_ps:
    mov rax, 0x1E
    int 80h
    ret

sys_changeProcessStatus:
    mov rax, 0x1F
    int 80h
    ret

sys_getCurrentPid:
    mov rax, 0x20
    int 80h
    ret

sys_exec:
    mov rax, 0x21
    int 80h
    ret

sys_exit:
    mov rax, 0x22
    int 80h
    ret

sys_waitpid:
    mov rax, 0x23
    int 80h
    ret

sys_kill:
    mov rax, 0x24
    int 80h
    ret

sys_block:
    mov rax, 0x25
    int 80h
    ret

sys_unblock:
    mov rax, 0x26
    int 80h
    ret

sys_getPriority:
    mov rax, 0x27
    int 80h
    ret