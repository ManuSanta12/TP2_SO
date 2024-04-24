GLOBAL ex_invopcode
GLOBAL ex_zerodiv

section .text

ex_invopcode:
	mov cr6, rax
	;ret

ex_zerodiv:
	mov rax, 0
	div rax
	;ret

section .bss