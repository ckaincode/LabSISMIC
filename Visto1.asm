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
; I = 0x0001, V= 0x0005, X = 0x0000A, L= 0x0032, C= 0x0074, M= 0x03E8
NUM			.equ	1488
;

			mov #NUM,R5
			mov #RESP,R6
			call #ALG_ROM
			jmp $
			nop
;
ALG_ROM:	cmp #0,R5
			jz END
			mov #1000,R7
			cmp R7,R5
			jge MAIOR1000			;rotina para maior ou igual a 1000
			jl	MENOR1000
			ret

MENOR1000:	cmp #900,R5
			jn MENOR900
			mov.b #0x43, 0(R6)
			inc R6
			mov.b #0x4D, 0(R6)
			inc R6
			sub #900,R5
			jmp ALG_ROM

			ret

MAIOR1000:
			mov.b #0x4d,0(R6)
			inc R6
			sub #1000,R5
			jmp ALG_ROM
			ret
MENOR900:
			cmp #500,R5
			jn MENOR500
			mov.b #0x44,0(R6)
			inc R6
			sub #500,R5
			jmp ALG_ROM

			ret

MENOR500:
			cmp #400,R5
			jn MENOR400
			mov.b #0x43,0(R6)
			inc R6
			mov.b #0x44,0(R6)
			inc R6
			sub #400,R5
			jmp ALG_ROM
			ret

MENOR400:
			cmp #100,R5
			jn MENOR100
			mov.b #0x43, 0(R6)
			inc R6
			sub #100, R5
			jmp ALG_ROM

MENOR100:
			cmp #90,R5
			jn MENOR90
			mov.b #0x58, 0(R6)
			inc R6
			mov.b #0x43, 0(R6)
			inc R6
			sub #90,R5
			jmp ALG_ROM

MENOR90:
			cmp #50,R5
			jn MENOR50
			mov.b #0x4C, 0(R6)
			inc R6
			sub #50,R5
			jmp ALG_ROM

MENOR50:
			cmp #40,R5
			jn MENOR40
			mov.b #0x58, 0(R6)
			inc R6
			mov.b #0x4C, 0(R6)
			inc R6
			sub #40,R5
			jmp ALG_ROM

MENOR40:
			cmp #10,R5
			jn MENOR10
			mov.b #0x58, 0(R6)
			inc R6
			sub #10,R5
			jmp ALG_ROM

MENOR10:
			cmp #9, R5
			jn MENOR9
			mov.b #0x49, 0(R6)
			inc R6
			mov.b #0x58, 0(R6)
			inc R6
			sub #9,R5
			jmp ALG_ROM

MENOR9:
			cmp #5,R5
			jn MENOR5
			mov.b #0x56, 0(R6)
			inc R6
			sub #5,R5
			jmp ALG_ROM

MENOR5:
			cmp #4,R5
			jn MENOR4
			mov.b #0x49, 0(R6)
			inc R6
			mov.b #0x56, 0(R6)
			inc R6
			sub #4,R5
			jmp ALG_ROM

MENOR4:
			cmp #1,R5
			jn END
			mov.b #0x49,0(R6)
			inc R6
			sub #1,R5
			jmp ALG_ROM

END:
			mov.b #0x00,0(R6)
			ret

			.data
			.retain
RESP:		.byte	"RRRRRRRRRRRRRRRRRR",0
                                            

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
            
