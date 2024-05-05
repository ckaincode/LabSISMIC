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
			mov #vetor, R5
			call #MAIOR16
			jmp $

MAIOR16:
			mov @R5+,R9
			mov #0,R7
			mov @R5,R6
			jmp LOOP
			ret

LOOP:
			dec R9
			jn END
			mov @R5+,R8
			cmp R8,R6
			jn SWAP
			jz EQUAL
			jmp LOOP
			ret

SWAP:
			mov R8,R6
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

vetor: 		.byte 6,0, "JOAQUIMJOSE",0
                                            

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
            
