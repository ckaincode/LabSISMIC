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

void debounce(long x){
    volatile long i;
    for(i = 0; i < x; i++);
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    config_leds();
    config_chaves();

    while (2>1){
        debounce(1000);
        while ((P2IN & BIT1) && (P1IN & BIT1));
        P1OUT ^= BIT0;
        debounce(1000);
        while (!(P2IN & BIT1) || !(P1IN & BIT1));
    }
    return 0;
}
