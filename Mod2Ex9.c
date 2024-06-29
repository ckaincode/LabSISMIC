#include <msp430.h> 

void config_leds(void){
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
}

void config_chaves(void){
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;
}

void config_timers(void){
    TA0CTL = TASSEL__SMCLK | MC__UP;
    TA0CCR0 = 8192;
    TA0CCR1 = 4096;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    config_leds();
    config_timers();

    while (2>1){
        while (!(TA0CCTL1 & CCIFG));
        TA0CCTL1 &= ~CCIFG;
        P1OUT &= ~BIT0;

        while (!(TA0CTL & TAIFG));
        TA0CTL &= ~TAIFG;
        P1OUT |= BIT0;
    }
    return 0;
}
