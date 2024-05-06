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
SW_LEDS:	call #CONFIG

LB1:		bit.b #BIT1,&P2IN
			jz LB2
			bic.b #BIT0,&P1OUT
			jmp LB3

LB2:		bis.b #BIT0,&P1OUT

LB3:		bit.b #BIT1, &P1IN
			jz LB4
			bic.b #BIT7, &P4OUT
			jmp LB1

LB4:		bis.b #BIT7,&P4OUT
			jmp LB1

CONFIG:		bis.b #BIT0, &P1DIR
			bic.b #BIT0, &P1OUT
			bis.b #BIT7,&P4DIR
			bic.b #BIT7,&P4OUT
			bic.b #BIT1,&P2DIR
			bis.b #BIT1,&P2REN
			bis.b #BIT1, &P2OUT
			bic.b #BIT1, &P1DIR
			bis.b #BIT1,&P1REN
			bis.b #BIT1,&P1OUT
			jmp LB1
			ret
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
            
