#include <msp430.h>
#define TRUE 1
#define FALSE 0


void configleds(void){
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
}


void timera0config(void){
    TA0CTL = TASSEL__SMCLK | MC__UP | TAIE;
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;
    TA0CCR0 = 8192;
    TA0CCR1 = 2457;
    TA0CCR2 = 5734;
}


#pragma vector = TIMER0_A1_VECTOR
__interrupt void led(void){
    int ivec = TA0IV;
    switch(ivec){
    case 2: P1OUT &= ~BIT0; break;
    case 4: P4OUT &= ~BIT7; break;
    case 14:
        P1OUT |= BIT0;
        P4OUT |= BIT7;
        break;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    configleds();
    timera0config();
    __enable_interrupt();
    while(TRUE);

    return 0;
}
