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
    TA0CTL = TASSEL__SMCLK | MC__UP | TAIE;
    TA0CCTL1 = CCIE;
    TA0CCTL2 = CCIE;

    TA0CCR0 = 8192;
    TA0CCR1 = 2457;
    TA0CCR2 = 5734;
}

void debounce(void){
    volatile int i = 10000;
    while (i--);
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void led(void){
    int seila = TA0IV;
    if (seila == 2){
        P1OUT &= ~BIT0;
    }
    if (seila == 4){
        P4OUT &= ~BIT7;
    }
    if (seila == 14){
        P1OUT |= BIT0;
        P4OUT |= BIT7;
    }
}

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    config_leds();
    config_chaves();
    config_timers();
    __enable_interrupt();

    while (2>1);
    return 0;
}
