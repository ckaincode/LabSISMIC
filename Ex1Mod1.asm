;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file
            
;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer


;-------------------------------------------------------------------------------
; Main loop here
;-------------------------------------------------------------------------------

; R5 -> ponteiro para o vetor, R6 retorna o menor elemento e R7 a sua frequência, R8 -> Letra a ser comparada
; R9 -> Contador


			mov #vetor, R5
			call #MENOR
			jmp $

MENOR:
			mov.b @R5+,R9
			mov #0,R7
			mov.b @R5,R6
			jmp LOOP
			ret

LOOP:
			dec.b R9
			jn END
			mov.b @R5+,R8
			cmp.b R6,R8
			jn SWAP
			jz EQUAL
			jmp LOOP
			ret

SWAP:
			mov.b R8,R6
			mov #1,R7
			jmp LOOP
			ret
EQUAL:
			inc R7
			jmp LOOP
			ret

END:
			ret


			.data
			.retain
vetor: 		.byte 11, "JOAQUIMJOSA"
;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack
            
;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
            
