#include <msp430.h>
#define TRUE 1

void config_leds(void){
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
}


void config_timers(void){
    TA0CTL = TASSEL__SMCLK | MC__UP;
    TA0CCTL1 = OUTMOD_7;
    TA0CCR0 = 8192;
    TA0CCR1 = 4096;
    P1SEL |= BIT2;
    P1DIR |= BIT2;
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    config_leds();
    config_timers();

    while (TRUE){
    }
    return 0;
}
